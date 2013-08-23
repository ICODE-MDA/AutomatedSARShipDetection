//----------------------------------------------------------------------------
//
// File: ossimPdfReader.cpp
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: OSSIM LAS LIDAR reader.
//
//----------------------------------------------------------------------------
// $Id: ossimPdfReader.cpp 2682 2011-06-06 17:55:17Z david.burken $

#include "ossimPdfReader.h"

#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimImageGeometryRegistry.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/projection/ossimProjectionFactoryRegistry.h>

#include <fstream>
#include <limits>
#include <sstream>
#include <string>

RTTI_DEF1(ossimPdfReader, "ossimPdfReader", ossimImageHandler)

static ossimTrace traceDebug("ossimPdfReader:debug");

static const char SCALE_KW[] = "scale";
static const char SCAN_KW[]  = "scan"; // boolean

ossimPdfReader::ossimPdfReader()
   : ossimImageHandler(),
     m_str(),
     m_mutex()
{
}

ossimPdfReader::~ossimPdfReader()
{
   close();
}

bool ossimPdfReader::open()
{
   static const char M[] = "ossimPdf::open";
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << M << " entered...\nfile: " << theImageFile << "\n";
   }
   
   bool result = false;

   close();

   m_str.open( theImageFile.c_str(), std::ios_base::in | std::ios_base::binary );
   if ( checkSignature() )
   {
      if ( initXrefTable() )
      {
         // Go through each object in the cross reference table.
         std::vector<std::streamoff>::const_iterator i = m_xref.begin();
         while ( i != m_xref.end() )
         {
            m_str.seekg( (*i), std::ios_base::beg );
            
            ++i;
         }
         
      }
   }

   if ( !result ) close();

   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << M << " exit status = " << (result?"true\n":"false\n");
   }
   
   return result;
}

void ossimPdfReader::completeOpen()
{
   establishDecimationFactors();
}

void ossimPdfReader::close()
{
   if ( isOpen() )
   {
      m_str.close();
      ossimImageHandler::close();
   }
}

ossimRefPtr<ossimImageData> ossimPdfReader::getTile(
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

bool ossimPdfReader::getTile(ossimImageData* result, ossim_uint32 resLevel)
{
   // static const char MODULE[] = "ossimLibLasReader::getTile(ossimImageData*, level)";

   bool status = false;

   return status;
   
} // End: bool ossimLibLasReader::getTile(ossimImageData* result, ossim_uint32 resLevel)

ossim_uint32 ossimPdfReader::getNumberOfInputBands() const
{
   return 1; // tmp
}

ossim_uint32 ossimPdfReader::getNumberOfLines(ossim_uint32 resLevel) const
{
   ossim_uint32 result = 0;
   if ( isOpen() )
   {
   }
   return result;
}

ossim_uint32 ossimPdfReader::getNumberOfSamples(ossim_uint32 resLevel) const
{
   ossim_uint32 result = 0;
   if ( isOpen() )
   {
   }
   return result;
}

ossim_uint32 ossimPdfReader::getImageTileWidth() const
{
   return 0;
}

ossim_uint32 ossimPdfReader::getImageTileHeight() const
{
   return 0;
}

ossim_uint32 ossimPdfReader::getTileWidth() const
{
   ossimIpt ipt;
   ossim::defaultTileSize(ipt);
   return ipt.x;
}

ossim_uint32 ossimPdfReader::getTileHeight() const
{
   ossimIpt ipt;
   ossim::defaultTileSize(ipt);
   return ipt.y; 
}

ossimScalarType ossimPdfReader::getOutputScalarType() const
{
   return OSSIM_UINT8; // tmp drb
}

void ossimPdfReader::getEntryList(std::vector<ossim_uint32>& entryList)const
{
   if ( isOpen() )
   {
   }
   else
   {
      entryList.clear();
   }
}

ossim_uint32 ossimPdfReader::getCurrentEntry() const
{
   return 0; // tmp drb
}

bool ossimPdfReader::setCurrentEntry(ossim_uint32 entryIdx)
{
   bool result = false;
   // tmp drb
   return result;
}

ossimString ossimPdfReader::getShortName() const
{
   return ossimString("pdf");
}
   
ossimString ossimPdfReader::getLongName()  const
{
   return ossimString("ossim pdf reader");
}

ossimRefPtr<ossimImageGeometry> ossimPdfReader::getImageGeometry()
{
   if ( !theGeometry )
   {
      // Check for external geom:
      theGeometry = getExternalImageGeometry();
      
      if ( !theGeometry )
      {
         theGeometry = new ossimImageGeometry();
         // tmp drb
         // if ( m_proj.valid() )
         // {
         //    theGeometry->setProjection( m_proj.get() );
         // }
         // else
         // {
         //    //---
         //    // WARNING:
         //    // Must create/set theGeometry at this point or the next call to 
         //    // ossimImageGeometryRegistry::extendGeometry will put us in an infinite loop
         //    // as it does a recursive call back to ossimImageHandler::getImageGeometry().
         //    //---         

         //    // Try factories for projection.
         //    ossimImageGeometryRegistry::instance()->extendGeometry(this);
         // }
      }
      
      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
   }
   
   return theGeometry;
}

double ossimPdfReader::getMinPixelValue(ossim_uint32 /* band */) const
{
   return 1; // tmp drb
}

double ossimPdfReader::getMaxPixelValue(ossim_uint32 /* band */) const
{
   return 255; // tmp drb
}

double ossimPdfReader::getNullPixelValue(ossim_uint32 /* band */) const
{
   return 0; // tmp drb
}

ossim_uint32 ossimPdfReader::getNumberOfDecimationLevels() const
{
   return 1;  // tmp drb
}

bool ossimPdfReader::saveState(ossimKeywordlist& kwl, const char* prefix)const
{
   return ossimImageHandler::saveState(kwl, prefix);
}

bool ossimPdfReader::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   bool result = ossimImageHandler::loadState(kwl, prefix);
   return result;
}

void ossimPdfReader::setProperty(ossimRefPtr<ossimProperty> property)
{
   if ( property.valid() )
   {
   }
}

ossimRefPtr<ossimProperty> ossimPdfReader::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> prop = 0;
   return prop;
}

void ossimPdfReader::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   ossimImageHandler::getPropertyNames(propertyNames);
}

bool ossimPdfReader::checkSignature()
{
   cout << "a..." << endl;
   bool result = false;

   if ( m_str.is_open() )
   {
      cout << "b..." << endl;
      
      // Look for "%PDF-1.7" at first line of file.
      std::string line;

      std::getline( m_str, line );

      cout << "first line: " << line << std::endl;
      
      if ( line.size() >= 8 )
      {
         std::string s = line.substr( 0, 4 );
         if ( s == "%PDF" )
         {
            // Seek to the end and look for "%%EOF".

            // Start 6 characters back and find the first newline 0x0a('\n').
            std::streamoff offset = -6;
            m_str.seekg( offset, std::ios_base::end );
            while ( m_str.peek() != 0x0a )
            {
               --offset;
               m_str.seekg( offset, std::ios_base::end );
            }

            // Seek past the newline character.
            m_str.seekg( 1, std::ios_base::cur );
               
            std::getline( m_str, line );
            if ( line.size() >= 5 )
            {
               s = line.substr( 0, 5 );
               if ( s == "%%EOF" )
               {
                  result = true;
               }
            }
            cout << "last line: " << line << endl;
         }
      }
   }
   
   return result;
}

bool ossimPdfReader::initXrefTable()
{
   bool result = false;
   m_xref.clear();
   
   cout << "c..." << endl;
   
   if ( m_str.is_open() )
   {
      cout << "d..." << endl;
      
      //---
      // Find the "startxref" at the end of file.  Typically it's:
      // startxref
      // offset
      // %%EOF
      //---

      //---
      // Start 6 characters back and find the first newline 0x0a('\n').
      // This is the end of offset line.
      //---
      std::streamoff offset = -6;
      m_str.seekg( offset, std::ios_base::end );
      while ( (m_str.peek() != 0x0a) && (m_str.peek() != 0x0d) )
      {
         --offset;
         m_str.seekg( offset, std::ios_base::end );
      }
      
      --offset; // Move past newline.
      m_str.seekg( offset, std::ios_base::end );

      // Find the next newline.  This is the end of the "startxref line.
      while ( (m_str.peek() != 0x0a) && (m_str.peek() != 0x0d) )
      {
         --offset;
         m_str.seekg( offset, std::ios_base::end );
      }

      --offset; // Move past newline.
      m_str.seekg( offset, std::ios_base::end );

      // Find the next newline.  This is the end of the fourth to last line.
      while ( (m_str.peek() != 0x0a) && (m_str.peek() != 0x0d) )
      {
         --offset;
         m_str.seekg( offset, std::ios_base::end );
      }

      cout << "tellg: " << m_str.tellg() << endl;

      // Seek to start of "startx" line:
      m_str.seekg( 1, std::ios_base::cur );

      // Grab the "startx" line.
      std::string line;
      std::getline( m_str, line );

      cout << "startxref line: " << line << std::endl;

      if ( line == "startxref" )
      {
         // Grab the offset line:
         std::getline( m_str, line );
         offset = ossimString(line).toInt32();

         // Seek to the cross reference block.

         cout << "offset: " << offset << endl;

         m_str.seekg( offset, std::ios_base::beg );

         // Grab the "xref" line:
         std::getline( m_str, line );

         cout << "xref line: " << line << endl;

         if ( line == "xref" )
         {
            ossim_int32 startOffset = 0;
            ossim_int32 objectCount = 0;

            m_str >> startOffset >> objectCount;
            

            for ( ossim_int32 i = 0; i < objectCount; ++i )
            {
               ossim_int64 objectOffset = 0;
               ossim_int32 revision = 0;  
               char usedMarker = '\0';

               m_str >> objectOffset >> revision >> usedMarker;

               cout << "objectOffset: " << objectOffset << endl;

               if ( usedMarker == 'n' )
               {
                  offset = startOffset + objectOffset;
                  m_xref.push_back( offset );
               }
            }

            result = m_xref.size() ? true : false;
         }
      }
   }
   return result;
   
} // End: bool ossimPdfReader::initXrefTable()

bool ossimPdfReader::backupLine( ossim_int32 atMostBytes )
{
   bool result = false;

   std::streamoff startPos = m_str.tellg();

   cout << "End byte tellg: " << startPos << std::endl;
   
   if ( startPos )
   {
      std::streamoff offset = 1;
      char c;
      m_str.get( c );
      cout << "c: " << int(c) << std::endl;
      
      while ( ( c != '\n') && ( offset <= atMostBytes ) )
      {
         
         m_str.seekg( startPos-offset, std::ios_base::beg );
         m_str.get( c );
         cout << "c: " << int(c) << std::endl;
         ++offset;
      }
   }
   return result;
}
