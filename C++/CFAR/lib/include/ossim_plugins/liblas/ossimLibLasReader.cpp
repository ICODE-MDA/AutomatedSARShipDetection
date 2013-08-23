//----------------------------------------------------------------------------
//
// File: ossimLibLasReader.cpp
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: OSSIM libLAS LIDAR reader.
//
//----------------------------------------------------------------------------
// $Id: ossimLibLasReader.cpp 2682 2011-06-06 17:55:17Z david.burken $

#include "ossimLibLasReader.h"

#include <ossim/base/ossimBooleanProperty.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageGeometryRegistry.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>
#include <ossim/support_data/ossimFgdcTxtDoc.h>
#include <ossim/support_data/ossimTiffInfo.h>

#include <liblas/header.hpp>

#include <exception>
#include <fstream>
#include <limits>
#include <sstream>

RTTI_DEF1(ossimLibLasReader, "ossimLibLasReader", ossimImageHandler)

static ossimTrace traceDebug("ossimLibLasReader:debug");
// static ossimTrace traceDump("ossimLibLasReader:dump");

static const char SCALE_KW[] = "scale";
static const char SCAN_KW[]  = "scan"; // boolean


ossimLibLasReader::ossimLibLasReader()
   : ossimImageHandler(),
     m_str(),
     m_rdr(0),
     m_proj(0),
     m_ul(),
     m_lr(),
     m_maxZ(0.0),
     m_minZ(0.0),
     m_scale(),
     m_tile(0),
     m_entry(0),
     m_mutex(),
     m_scan(true), // ???
     m_units(OSSIM_METERS),
     m_unitConverter(0)
{
   //---
   // Nan out as can be set in several places, i.e. setProperty,
   // loadState and initProjection.
   //---
   m_scale.makeNan();
}

ossimLibLasReader::~ossimLibLasReader()
{
   close();
}

bool ossimLibLasReader::open()
{
   static const char M[] = "ossimLibLas::open";
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << M << " entered...\nfile: " << theImageFile << "\n";
   }
   
   bool result = false;

   close();

   m_str.open(theImageFile.c_str(), std::ios_base::in | std::ios_base::binary);
   if ( checkSignature() )
   {
      try
      {
         m_rdr = new liblas::Reader(m_str);

         result = init();

         if ( result )
         {
            establishDecimationFactors();

            if ( traceDebug() )
            {
               const liblas::Header& hdr = m_rdr->GetHeader();
               ossimNotify(ossimNotifyLevel_DEBUG) << hdr << "\n";
            }
         }
         else
         {
            close();
         }

#if 0 /* Please leave for debug. (drb) */
         if ( traceDump() )
         {
            m_rdr->Reset();
            while ( m_rdr->ReadNextPoint() )
            {
               liblas::Point const& p = m_rdr->GetPoint();
               ossimNotify(ossimNotifyLevel_DEBUG) << p << " ";
            }
            ossimNotify(ossimNotifyLevel_DEBUG) << "\n";
         }
#endif
      }
      catch ( const std::exception& e )
      {
         ossimNotify(ossimNotifyLevel_WARN)
            << "Caught exception from liblas::Reader: " << e.what() << std::endl;
         close();
         result = false;
      }
   }
   else
   {
      m_str.close();
   }

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exit status = " << (result?"true\n":"false\n");
   }
   
   return result;
}

void ossimLibLasReader::completeOpen()
{
   establishDecimationFactors();
}

void ossimLibLasReader::close()
{
   m_entry = 0;
   m_tile  = 0;
   m_proj  = 0;
   if ( m_unitConverter )
   {
      delete m_unitConverter;
      m_unitConverter = 0;
   }
   if ( m_rdr )
   {
      delete m_rdr;
      m_rdr = 0;
   }
   if ( isOpen() )
   {
      m_str.close();
      ossimImageHandler::close();
   }
}

ossimRefPtr<ossimImageData> ossimLibLasReader::getTile(
   const  ossimIrect& tile_rect, ossim_uint32 resLevel)
{
   if ( m_tile.valid() )
   {
      // Image rectangle must be set prior to calling getTile.
      m_tile->setImageRectangle(tile_rect);

      if ( getTile( m_tile.get(), resLevel ) == false )
      {
         if (m_tile->getDataObjectStatus() != OSSIM_NULL)
         {
            m_tile->makeBlank();
         }
      }
   }
   return m_tile;
}

bool ossimLibLasReader::getTile(ossimImageData* result, ossim_uint32 resLevel)
{
   bool status = false;

   if ( m_rdr && result && (result->getScalarType() == OSSIM_FLOAT32) &&
        (result->getDataObjectStatus() != OSSIM_NULL) &&
        !m_ul.hasNans() && !m_scale.hasNans() )
   {
      status = true;
      
      const ossimIrect  TILE_RECT   = result->getImageRectangle();
      const ossim_int32 TILE_HEIGHT = static_cast<ossim_int32>(TILE_RECT.height());
      const ossim_int32 TILE_WIDTH  = static_cast<ossim_int32>(TILE_RECT.width());
      const ossim_int32 TILE_SIZE   = static_cast<ossim_int32>(TILE_RECT.area());

      const ossim_uint16 ENTRY = m_entry+1;

      // Get the scale for this resLevel:
      ossimDpt scale;
      getScale(scale, resLevel);
      
      // Set the starting upper left of upper left pixel for this tile.
      const ossimDpt UL_PROG_PT( m_ul.x - scale.x / 2.0 + TILE_RECT.ul().x * scale.x,
                                 m_ul.y + scale.y / 2.0 - TILE_RECT.ul().y * scale.y);

      //---
      // Set the lower right to the edge of the tile boundary.  This looks like an
      // "off by one" error but it's not.  We want the ossimDrect::pointWithin to
      // catch any points in the last line sample.
      //---
      const ossimDpt LR_PROG_PT( UL_PROG_PT.x + TILE_WIDTH  * scale.x,
                                 UL_PROG_PT.y - TILE_HEIGHT * scale.y);
      
      const ossimDrect PROJ_RECT(UL_PROG_PT, LR_PROG_PT, OSSIM_RIGHT_HANDED);

#if 0  /* Please leave for debug. (drb) */
      cout << "m_ul: " << m_ul
           << "\nm_scale: " << m_scale
           << "\nscale:   " << scale
           << "\nresult->getScalarType(): " << result->getScalarType()
           << "\nresult->getDataObjectStatus(): " << result->getDataObjectStatus()
           << "\nPROJ_RECT: " << PROJ_RECT
           << "\nTILE_RECT: " << TILE_RECT
           << "\nUL_PROG_PT: " << UL_PROG_PT << endl;
#endif

      // Create array of buckets.
      std::vector<ossimLibLasReader::Bucket> bucket( TILE_SIZE );

      // Loop through the point data.
      m_rdr->Reset();
      ossimDpt lasPt;
      while ( m_rdr->ReadNextPoint() )
      {
         if ( m_rdr->GetPoint().GetReturnNumber() == ENTRY )
         {
            lasPt.x = m_rdr->GetPoint().GetX();
            lasPt.y = m_rdr->GetPoint().GetY();
            if ( m_unitConverter )
            {
               convertToMeters(lasPt.x);
               convertToMeters(lasPt.y);
            }
            
            if ( PROJ_RECT.pointWithin( lasPt ) )
            {
               // Compute the bucket index:
               ossim_int32 line = static_cast<ossim_int32>((UL_PROG_PT.y - lasPt.y) / scale.y);
               ossim_int32 samp = static_cast<ossim_int32>((lasPt.x - UL_PROG_PT.x) / scale.x );
               ossim_int32 bucketIndex = line * TILE_WIDTH + samp;
               
               // Range check and add if in there.
               if ( ( bucketIndex >= 0 ) && ( bucketIndex < TILE_SIZE ) )
               {
                  ossim_float64 z = m_rdr->GetPoint().GetZ();
                  if ( m_unitConverter ) convertToMeters(z);
                  bucket[bucketIndex].add( z ); 
               }
            }
         }
      } 

      //---
      // We must always blank out the tile as we may not have a point for every
      // point.
      //---
      result->makeBlank();

      ossim_float32* buf = result->getFloatBuf(); // Tile buffer to fill.
      
      // Fill the tile.  Currently no band loop:
      for (ossim_int32 i = 0; i < TILE_SIZE; ++i)
      {
         buf[i] = bucket[i].getValue();
      }

      // Revalidate.
      result->validate();
   }

   return status;
   
} // End: bool ossimLibLasReader::getTile(ossimImageData* result, ossim_uint32 resLevel)

ossim_uint32 ossimLibLasReader::getNumberOfInputBands() const
{
   return 1; // tmp
}

ossim_uint32 ossimLibLasReader::getNumberOfLines(ossim_uint32 resLevel) const
{
   ossim_uint32 result = 0;
   if ( isOpen() )
   {
      result = static_cast<ossim_uint32>(ceil(m_ul.y - m_lr.y) / m_scale.y);
      if (resLevel) result = (result>>resLevel);
   }
   return result;
}

ossim_uint32 ossimLibLasReader::getNumberOfSamples(ossim_uint32 resLevel) const
{
   ossim_uint32 result = 0;
   if ( isOpen() )
   {
      result = static_cast<ossim_uint32>(ceil(m_lr.x - m_ul.x) / m_scale.x);
      if (resLevel) result = (result>>resLevel);
   }
   return result;
}

ossim_uint32 ossimLibLasReader::getImageTileWidth() const
{
   return 0;
}

ossim_uint32 ossimLibLasReader::getImageTileHeight() const
{
   return 0; 
}

ossim_uint32 ossimLibLasReader::getTileWidth() const
{
   ossimIpt ipt;
   ossim::defaultTileSize(ipt);
   return ipt.x;
}

ossim_uint32 ossimLibLasReader::getTileHeight() const
{
   ossimIpt ipt;
   ossim::defaultTileSize(ipt);
   return ipt.y; 
}

ossimScalarType ossimLibLasReader::getOutputScalarType() const
{
   return OSSIM_FLOAT32;
}

void ossimLibLasReader::getEntryList(std::vector<ossim_uint32>& entryList)const
{
   if ( isOpen() )
   {
      ossim_uint32 entry = 0;
      liblas::Header::RecordsByReturnArray::const_iterator i =
         m_rdr->GetHeader().GetPointRecordsByReturnCount().begin();
      while ( i != m_rdr->GetHeader().GetPointRecordsByReturnCount().end() )
      {
         if ( (*i) > 0 )
         {
            entryList.push_back(entry); // Only count entries that have points.
         }
         ++entry;
         ++i;
      }
   }
   else
   {
      entryList.clear();
   }
}

ossim_uint32 ossimLibLasReader::getCurrentEntry() const
{
   return static_cast<ossim_uint32>(m_entry);
}

bool ossimLibLasReader::setCurrentEntry(ossim_uint32 entryIdx)
{
   bool result = false;
   if ( m_entry != entryIdx)
   {
      if ( isOpen() )
      {
         std::vector<ossim_uint32> entryList;
         getEntryList( entryList );
         std::vector<ossim_uint32>::const_iterator i = entryList.begin();
         while ( i != entryList.end() )
         {
            if ( (*i) == entryIdx )
            {
               m_entry = entryIdx;
               result = true;
            }
            ++i;
         }
      }
   }
   return result;
}

ossimString ossimLibLasReader::getShortName() const
{
   return ossimString("las");
}
   
ossimString ossimLibLasReader::getLongName()  const
{
   return ossimString("ossim las (liblas) reader");
}

ossimRefPtr<ossimImageGeometry> ossimLibLasReader::getImageGeometry()
{
   if ( !theGeometry )
   {
      // Check for external geom:
      theGeometry = getExternalImageGeometry();
      
      if ( !theGeometry )
      {
         theGeometry = new ossimImageGeometry();
         if ( m_proj.valid() )
         {
            theGeometry->setProjection( m_proj.get() );
         }
         else
         {
            //---
            // WARNING:
            // Must create/set theGeometry at this point or the next call to 
            // ossimImageGeometryRegistry::extendGeometry will put us in an infinite loop
            // as it does a recursive call back to ossimImageHandler::getImageGeometry().
            //---         

            // Try factories for projection.
            ossimImageGeometryRegistry::instance()->extendGeometry(this);
         }
      }
      
      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
   }
   
   return theGeometry;
}

double ossimLibLasReader::getMinPixelValue(ossim_uint32 /* band */) const
{
   return m_minZ;
}

double ossimLibLasReader::getMaxPixelValue(ossim_uint32 /* band */) const
{
   return m_maxZ;
}

double ossimLibLasReader::getNullPixelValue(ossim_uint32 /* band */) const
{
   return -99999.0;
}

ossim_uint32 ossimLibLasReader::getNumberOfDecimationLevels() const
{
   // Can support any number of rlevels.
   ossim_uint32 result = 1;
   const ossim_uint32 STOP_DIMENSION = 16;
   ossim_uint32 largestImageDimension = getNumberOfSamples(0) > getNumberOfLines(0) ?
      getNumberOfSamples(0) : getNumberOfLines(0);
   while(largestImageDimension > STOP_DIMENSION)
   {
      largestImageDimension /= 2;
      ++result;
   }
   return result;
}

bool ossimLibLasReader::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   kwl.add( prefix, SCALE_KW, m_scale.toString().c_str(), true );
   kwl.add( prefix, SCAN_KW,  ossimString::toString(m_scan).c_str(), true );
   return ossimImageHandler::saveState(kwl, prefix);
}

bool ossimLibLasReader::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   bool result = false;
   if ( ossimImageHandler::loadState(kwl, prefix) )
   {
      result = open();
      if ( result )
      {
         // Get our keywords:
         const char* lookup = kwl.find(prefix, SCALE_KW);
         if ( lookup )
         {
            m_scale.toPoint( ossimString(lookup) );
         }
         lookup = kwl.find(prefix, SCAN_KW);
         if ( lookup )
         {
            ossimString s = lookup;
            m_scan = s.toBool();
         }
      }
   }
   return result;
}

void ossimLibLasReader::setProperty(ossimRefPtr<ossimProperty> property)
{
   if ( property.valid() )
   {
      if ( property->getName() == SCALE_KW )
      {
         ossimString s;
         property->valueToString(s);
         ossim_float64 d = s.toFloat64();
         if ( ossim::isnan(d) == false )
         {
            setScale( d );
         }
      }
      else if ( property->getName() == SCAN_KW )
      {
         ossimString s;
         property->valueToString(s);
         m_scan = s.toBool();
      }
      else
      {
         ossimImageHandler::setProperty(property);
      }
   }
}

ossimRefPtr<ossimProperty> ossimLibLasReader::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> prop = 0;
   if ( name == SCALE_KW )
   {
      ossimString value = ossimString::toString(m_scale.x);
      prop = new ossimStringProperty(name, value);
   }
   else if ( name == SCAN_KW )
   {
      prop = new ossimBooleanProperty(name, m_scan);
   }
   else
   {
      prop = ossimImageHandler::getProperty(name);
   }
   return prop;
}

void ossimLibLasReader::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   propertyNames.push_back( ossimString(SCALE_KW) );
   propertyNames.push_back( ossimString(SCAN_KW) );
   ossimImageHandler::getPropertyNames(propertyNames);
}

bool ossimLibLasReader::init()
{
   bool result = false;

   if ( isOpen() )
   {
      result = parseVarRecords();

      if ( !result )
      {
         result = initFromExternalMetadata(); // Checks for external FGDC text file.
      }
      
      // There is nothing we can do if parseVarRecords fails.
      if ( result )
      {
         initTile();
      }
   }
   
   return result;
}

bool ossimLibLasReader::initProjection()
{
   bool result = true;
   
   ossimMapProjection* proj = dynamic_cast<ossimMapProjection*>( m_proj.get() );
   if ( proj )
   {
      //---
      // Set the tie and scale:
      // Note the scale can be set in other places so only set here if it
      // has nans.
      //---
      if ( proj->isGeographic() )
      {
         ossimGpt gpt(m_ul.y, m_ul.x, 0.0, proj->getDatum() );
         proj->setUlTiePoints( gpt );

         if ( m_scale.hasNans() )
         {
            m_scale = proj->getDecimalDegreesPerPixel();
            if ( m_scale.hasNans() || !m_scale.x || !m_scale.y )
            {
               // Set to some default:
               m_scale.x = 0.000008983; // About 1 meter at the Equator.
               m_scale.y = m_scale.x;
               proj->setDecimalDegreesPerPixel( m_scale );
            }
         }
      }
      else
      {
         proj->setUlTiePoints(m_ul);

         if ( m_scale.hasNans() )
         {
            m_scale = proj->getMetersPerPixel();
            if ( m_scale.hasNans() || !m_scale.x || !m_scale.y )
            {
               // Set to some default:
               m_scale.x = 1.0;
               m_scale.y = 1.0;
               proj->setMetersPerPixel( m_scale );
            }
         }
      }
   }
   else
   {
      result = false;
      m_ul.makeNan();
      m_lr.makeNan();
      m_scale.makeNan();
      
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossimLibLasReader::initProjection WARN Could not cast to map projection!"
         << std::endl;
   }

   return result;
   
} // bool ossimLibLasReader::initProjection()

void ossimLibLasReader::initTile()
{
   const ossim_uint32 BANDS = getNumberOfOutputBands();

   m_tile = new ossimImageData(this,
                               getOutputScalarType(),
                               BANDS,
                               getTileWidth(),
                               getTileHeight());

   for(ossim_uint32 band = 0; band < BANDS; ++band)
   {
      m_tile->setMinPix(getMinPixelValue(band),   band);
      m_tile->setMaxPix(getMaxPixelValue(band),   band);
      m_tile->setNullPix(getNullPixelValue(band), band);
   }

   m_tile->initialize();
}

void ossimLibLasReader::initUnits(const ossimKeywordlist& geomKwl)
{
   ossimMapProjection* proj = dynamic_cast<ossimMapProjection*>( m_proj.get() );
   if ( proj )
   {
      if ( proj->isGeographic() )
      {
         m_units = OSSIM_DEGREES;
      }
      else
      {
         const char* lookup = geomKwl.find("image0.linear_units");
         if ( lookup )
         {
            std::string units = lookup;
            if ( units == "meters" )
            {
               m_units = OSSIM_METERS;
            }  
            else if ( units == "feet" )
            {
               m_units = OSSIM_FEET;
            }
            else if ( units == "us_survey_feet" )
            {
               m_units = OSSIM_US_SURVEY_FEET;
            }
            else
            {
               ossimNotify(ossimNotifyLevel_DEBUG)
                  << "ossimLibLasReader::initUnits WARN:\n"
                  << "Unhandled linear units code: " << units << std::endl;
            }
         }
      }
   }

   if ( m_units != OSSIM_METERS && !m_unitConverter )
   {
      m_unitConverter = new ossimUnitConversionTool();
   }
}

void ossimLibLasReader::initValues()
{
   static const char M[] = "ossimLibLasReader::initValues";
   
   if ( m_scan )
   {
      // Set to bogus values to start.
      m_ul.x = numeric_limits<ossim_float64>::max();
      m_ul.y = numeric_limits<ossim_float64>::min();
      m_lr.x = numeric_limits<ossim_float64>::min();
      m_lr.y = numeric_limits<ossim_float64>::max();
      m_maxZ = numeric_limits<ossim_float64>::min();
      m_minZ = numeric_limits<ossim_float64>::max();
      
      m_rdr->Reset();
      while ( m_rdr->ReadNextPoint() )
      {
         if ( m_rdr->GetPoint().GetX() < m_ul.x ) m_ul.x = m_rdr->GetPoint().GetX();
         if ( m_rdr->GetPoint().GetX() > m_lr.x ) m_lr.x = m_rdr->GetPoint().GetX();
         if ( m_rdr->GetPoint().GetY() > m_ul.y ) m_ul.y = m_rdr->GetPoint().GetY();
         if ( m_rdr->GetPoint().GetY() < m_lr.y ) m_lr.y = m_rdr->GetPoint().GetY();
         if ( m_rdr->GetPoint().GetZ() > m_maxZ ) m_maxZ = m_rdr->GetPoint().GetZ();
         if ( m_rdr->GetPoint().GetZ() < m_minZ ) m_minZ = m_rdr->GetPoint().GetZ();
      }
      m_rdr->Reset();
   }
   else
   {
      // Set the upper left (tie).
      m_ul.x = m_rdr->GetHeader().GetMinX();
      m_ul.y = m_rdr->GetHeader().GetMaxY();
      
      // Set the lower right.
      m_lr.x = m_rdr->GetHeader().GetMaxX();
      m_lr.y = m_rdr->GetHeader().GetMinY();
      
      // Set the min/max:
      m_maxZ = m_rdr->GetHeader().GetMaxZ();
      m_minZ = m_rdr->GetHeader().GetMinZ();
   }
   
   if ( m_unitConverter ) // Need to convert to meters.
   {
      convertToMeters(m_ul.x);
      convertToMeters(m_ul.y);
      
      convertToMeters(m_lr.x);
      convertToMeters(m_lr.y);
      
      convertToMeters(m_maxZ);
      convertToMeters(m_minZ);
   }
   
   if ( traceDebug() )
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " DEBUG:\nBounds from header:";
      ossimDpt pt;
      pt.x = m_rdr->GetHeader().GetMinX();
      pt.y = m_rdr->GetHeader().GetMaxY();
      if ( m_unitConverter )
      {
         convertToMeters(pt.x);
         convertToMeters(pt.y);
      }
      ossimNotify(ossimNotifyLevel_DEBUG) << "\nul:   " << pt;
      pt.x = m_rdr->GetHeader().GetMaxX();
      pt.y = m_rdr->GetHeader().GetMinY();
      if ( m_unitConverter )
      {
         convertToMeters(pt.x);
         convertToMeters(pt.y); 
      }
      ossimNotify(ossimNotifyLevel_DEBUG) << "\nlr:   " << pt;
      pt.x = m_rdr->GetHeader().GetMinZ();
      pt.y = m_rdr->GetHeader().GetMaxZ();
      if ( m_unitConverter )
      {
         convertToMeters(pt.x);
         convertToMeters(pt.y); 
      }
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "\nminZ: " << pt.x
         << "\nmaxZ: " << pt.y << "\n";
      if ( m_scan )
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "Bounds from scan:"
            << "\nul:   " << m_ul
            << "\nlr:   " << m_lr
            << "\nminZ: " << m_minZ
            << "\nmaxZ: " << m_maxZ << "\n";
      }
   }
}

bool ossimLibLasReader::parseVarRecords()
{
   static const char M[] = "ossimLibLas::parseVarRecords";
   if ( traceDebug() ) ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   
   bool result = false;

   if ( isOpen() )
   {
      const liblas::Header& hdr = m_rdr->GetHeader();

      std::streampos origPos = m_str.tellg();
      std::streamoff pos = static_cast<std::streamoff>(hdr.GetHeaderSize());
      
      m_str.seekg(pos, std::ios_base::beg);

      ossim_uint32 vlrCount = hdr.GetVLRs().size();
      ossim_uint16 reserved;
      char uid[17];
      uid[16]='\n';
      ossim_uint16 recordId;
      ossim_uint16 length;
      char des[33];
      des[32] = '\n';

      //---
      // Things we need to save for printGeoKeys:
      //---
      ossim_uint16*  geoKeyBlock     = 0;
      ossim_uint64   geoKeyLength    = 0;
      ossim_float64* geoDoubleBlock  = 0;
      ossim_uint64   geoDoubleLength = 0;
      ossim_int8*    geoAsciiBlock   = 0;
      ossim_uint64   geoAsciiLength  = 0;
     
      ossimEndian* endian = 0;
      // LAS LITTLE ENDIAN:
      if ( ossim::byteOrder() == OSSIM_BIG_ENDIAN )
      {
         endian = new ossimEndian;
      }
      
      for ( ossim_uint32 i = 0; i < vlrCount; ++i )
      {
         m_str.read((char*)&reserved, 2);
         m_str.read(uid, 16);
         m_str.read((char*)&recordId, 2);
         m_str.read((char*)&length, 2);
         m_str.read(des, 32);

         // LAS LITTLE ENDIAN:
         if ( endian )
         {
            endian->swap(recordId);
            endian->swap(length);
         }

#if 0 /* Please leave for debug. (drb) */
         if ( traceDebug() )
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "uid:      " << uid
               << "\nrecordId: " << recordId
               << "\nlength:   " << length
               << "\ndes:      " << des
               << std::endl;
         }
#endif
         
         if (recordId == 34735) // GeoTiff projection keys.
         {
            geoKeyLength = length/2;
            geoKeyBlock = new ossim_uint16[geoKeyLength];
            m_str.read((char*)geoKeyBlock, length);
            
         }
         else if (recordId == 34736) // GeoTiff double parameters.
         {
            geoDoubleLength = length/8;
            geoDoubleBlock = new ossim_float64[geoDoubleLength];
            m_str.read((char*)geoDoubleBlock, length);
         }
         else if (recordId == 34737) // GeoTiff ascii block.
         {
            geoAsciiLength = length;
            geoAsciiBlock = new ossim_int8[length];
            m_str.read((char*)geoAsciiBlock, length);
         }
         else
         {
            m_str.seekg(length, std::ios_base::cur);
         }
      }

      //---
      // Must have the geoKeyBlock and a geoDoubleBlock for a projection.
      // Note the geoAsciiBlock is not needed, i.e. only informational.
      //---
      if ( geoKeyBlock && geoDoubleBlock )
      {
         if ( endian )
         {
            endian->swap(geoKeyBlock, geoKeyLength);
            endian->swap(geoDoubleBlock, geoDoubleLength);
         }

         //---
         // Give the geokeys to ossimTiffInfo to get back a keyword list that can be fed to
         // ossimProjectionFactoryRegistry::createProjection
         //---
         ossimTiffInfo info;
         ossimKeywordlist geomKwl;
         info.getImageGeometry(geoKeyLength, geoKeyBlock,
                               geoDoubleLength,geoDoubleBlock,
                               geoAsciiLength,geoAsciiBlock,
                               geomKwl);
            
         // Create the projection.
         m_proj = ossimProjectionFactoryRegistry::instance()->createProjection(geomKwl);
         if (m_proj.valid())
         {
            // Units must be set before initValues and initProjection.
            initUnits(geomKwl);

            // Must be called before initProjection.
            initValues();

            result = initProjection();  // Sets the ties and scale...

            if (traceDebug())
            {
               m_proj->print(ossimNotify(ossimNotifyLevel_DEBUG));
            }
         }
      }

      if ( geoKeyBlock )
      {
         delete [] geoKeyBlock;
         geoKeyBlock = 0;
      }      
      if (geoDoubleBlock)
      {
         delete [] geoDoubleBlock;
         geoDoubleBlock = 0;
      }
      if (geoAsciiBlock)
      {
         delete [] geoAsciiBlock;
         geoAsciiBlock = 0;
      }

      m_str.seekg(origPos);
   }  

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exit status = " << (result?"true\n":"false\n");
   }   
   return result;
}

bool ossimLibLasReader::initFromExternalMetadata()
{
   static const char M[] = "ossimLibLas::initFromExternalMetadata";
   if (traceDebug()) ossimNotify(ossimNotifyLevel_DEBUG) << M << " entered...\n";
   
   bool result = false;

   ossimFilename fgdcFile = theImageFile;
   fgdcFile.setExtension("txt");
   if ( fgdcFile.exists() == false )
   {
      fgdcFile.setExtension("TXT");
   }

   if ( fgdcFile.exists() )
   {
      ossimRefPtr<ossimFgdcTxtDoc> fgdcDoc = new ossimFgdcTxtDoc();
      if ( fgdcDoc->open( fgdcFile ) )
      {
         fgdcDoc->getProjection( m_proj );
         if ( m_proj.valid() )
         {
            // Units must be set before initValues and initProjection.
            std::string units;
            fgdcDoc->getAltitudeDistanceUnits(units);
            if ( ( units == "feet" ) || ( units == "international feet" ) )
            {
               m_units = OSSIM_FEET;
            }
            else if ( units == "survey feet" )
            {
               m_units = OSSIM_US_SURVEY_FEET;
            }
            else
            {
               m_units = OSSIM_METERS;
            }
            
            // Must be called before initProjection.
            initValues();
            
            result = initProjection();  // Sets the ties and scale...
            
            if (traceDebug())
            {
               m_proj->print(ossimNotify(ossimNotifyLevel_DEBUG));
            }
         }
      }
   }

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exit status = " << (result?"true\n":"false\n");
   }   
   return result;
}

bool ossimLibLasReader::checkSignature()
{
   bool result = false;
   if ( m_str.is_open() )
   {
      char SIG[4];
      m_str.read(SIG, 4);
      m_str.seekg(0, std::ios_base::beg);
      if ( (SIG[0] == 'L') && (SIG[1] == 'A') && (SIG[2] == 'S') && (SIG[3] == 'F') )
      {
         result = true;
      }
   }
   return result;
}

void ossimLibLasReader::getScale(ossimDpt& scale, ossim_uint32 resLevel) const
{
   // std::pow(2.0, 0) returns 1.
   ossim_float64 d = std::pow(2.0, static_cast<double>(resLevel));
   scale.x = m_scale.x * d;
   scale.y = m_scale.y * d;
}

void ossimLibLasReader::setScale( const ossim_float64& scale )
{
   m_scale.x = scale;
   m_scale.y = m_scale.x;

   if ( m_proj.valid() )
   {
      ossimMapProjection* proj = dynamic_cast<ossimMapProjection*>( m_proj.get() );
      if ( proj  && ( m_scale.hasNans() == false ) )
      {
         if ( proj->isGeographic() )
         {
            proj->setDecimalDegreesPerPixel( m_scale );
         }
         else
         {
            proj->setMetersPerPixel( m_scale );
         }
      }
   }
}
