//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  David Burken
//
// Description: OSSIM Portable Document Format (PDF) writer.
//
//----------------------------------------------------------------------------
// $Id$

#include "ossimPdfWriter.h"
#include <ossim/base/ossimBooleanProperty.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimEndian.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimScalarTypeLut.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimNumericProperty.h>
#include <ossim/imaging/ossimBandSelector.h>
#include <ossim/imaging/ossimGeneralRasterWriter.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/projection/ossimMapProjection.h>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>


RTTI_DEF1(ossimPdfWriter,
	  "ossimPdfWriter",
	  ossimImageFileWriter)

//---
// For trace debugging (to enable at runtime do:
// your_app -T "ossimPdfWriter:debug" your_app_args
//---
static ossimTrace traceDebug("ossimPdfWriter:debug");
static ossimTrace traceLog("ossimPdfWriter:log");

//---
// For the "ident" program which will find all exanded $Id$ macros and print them.
//---
#if OSSIM_ID_ENABLED
static const char OSSIM_ID[] = "$Id$";
#endif

ossimPdfWriter::ossimPdfWriter()
   : ossimImageFileWriter(),
     m_str(0),
     m_ownsStream(false),
     m_savedInput(0),
     m_kwl( new ossimKeywordlist() ),
     m_mutex()
{
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimPdfWriter::ossimPdfWriter entered" << std::endl;
#if OSSIM_ID_ENABLED
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "OSSIM_ID:  "
         << OSSIM_ID
         << std::endl;
#endif
   }
   
   // Since there is no internal geometry set the flag to write out one.
   setWriteExternalGeometryFlag(true);
   
   theOutputImageType = "ossim_pdf";
}

ossimPdfWriter::~ossimPdfWriter()
{
   // This will flush stream and delete it if we own it.
   close();

   m_kwl = 0; // Not a leak, ossimRefPtr
}

ossimString ossimPdfWriter::getShortName() const
{
   return ossimString("ossim_pdf_writer");
}

ossimString ossimPdfWriter::getLongName() const
{
   return ossimString("ossim pdf writer");
}

ossimString ossimPdfWriter::getClassName() const
{
   return ossimString("ossimPdfWriter");
}

bool ossimPdfWriter::writeFile()
{
   bool result = false;
   
   if( theInputConnection.valid() && ( getErrorStatus() == ossimErrorCodes::OSSIM_OK ) )
   {
      //---
      // Make sure we can open the file.  Note only the master process is used for
      // writing...
      //---
      if(theInputConnection->isMaster())
      {
         if (!isOpen())
         {
            open();
         }
      }
      
      result = writeStream();
   }

   return result;
}

bool ossimPdfWriter::writeStream()
{
   static const char MODULE[] = "ossimPdfWriter::writeStream";
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG) << MODULE << " entered..." << std::endl;
   }
   
   bool result = false;

   if ( isOpen() )
   {
      // Make sure we have a region of interest.
      if( theAreaOfInterest.hasNans() )
      {
         theInputConnection->initialize();
         theAreaOfInterest = theInputConnection->getAreaOfInterest();
      }
      else
      {
         theInputConnection->setAreaOfInterest( theAreaOfInterest );
      }
      
      // Cross reference(xref) table.  Holds the offset of each object.  Written at the end.
      std::vector<std::streamoff> xref;
      
      //---
      // Header:
      //---
      *m_str << "%PDF-1.7\n\n";
      
      //---
      // Catalog object:
      //---
      
      // Capture position:
      xref.push_back( (std::streamoff)m_str->tellp() );
      
      *m_str << "1 0 obj\n"
             << "  << /Type /Catalog\n"
             << "     /Pages 2 0 R\n"
             << "  >>\n"
             << "endobj\n\n";

      writeImage( m_str, xref );

      //---
      // Cross reference tables:
      //---

      // Capture start of xref for the second to last line:
      std::streamoff xrefOffset = (std::streamoff)m_str->tellp();

      writeXref( m_str, xref );

      //---
      // Trailer:
      //---
      writeTrailer( m_str, static_cast<ossim_uint32>(xref.size()+1), xrefOffset );

      close();

      // Set the status:
      result = true;

      // Reset the input to the image source sequence if it was modified.      
      if ( m_savedInput.get() != theInputConnection->getInput( 0 ) )
      {
         theInputConnection->connectMyInputTo( 0, m_savedInput.get() );  
      }
   }
   
   if (traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << MODULE << " exit status = " << (result?"true":"false") << std::endl;
   }

   return result;
   
} // bool ossimPdfWriter::writeStream()

void ossimPdfWriter::writeHeader( std::ostream* str )
{
   if ( str )
   {
      *str << "%PDF-1.7\n\n";
   }
}

void ossimPdfWriter::writeCatalog( std::ostream* str )
{
   if ( str )
   {
      *str << "1 0 obj\n"
           << "  << /Type /Catalog\n"
           << "     /Pages 2 0 R\n"
           << "  >>\n"
           << "endobj\n\n";
   }
}

void ossimPdfWriter::writePages( std::ostream* str,
                                 ossim_uint32 objectNumber,
                                 ossim_uint32 count  )
{
   if ( str )
   {
      *str << ossimString::toString( objectNumber ) << " 0 obj\n"
           << "  << /Type /Pages\n"
           << "     /Kids [" << ossimString::toString(objectNumber+1) << " 0 R]\n"
           << "     /Count " << ossimString::toString(count) << "\n"         
             << "  >>\n"
             << "endobj\n\n";
   }
}

bool ossimPdfWriter::writeImage( std::ostream* str,
                                 std::vector<std::streamoff>& xref )
{
   bool status = false;

   if ( str )
   {
      // Pages object:
      
      // Capture position:
      xref.push_back( (std::streamoff)m_str->tellp() );

      *m_str << xref.size() << " 0 obj\n"
             << "  << /Type /Pages\n"
             << "     /Kids [3 0 R]\n"
             << "     /Count 1\n"         
             << "  >>\n"
             << "endobj\n\n";

      // Page object:
      
      // Capture position:
      xref.push_back( (std::streamoff)m_str->tellp() );

      *m_str << xref.size() << " 0 obj % Page object\n"
             << "  << /Type /Page\n"
             << "     /Parent 2 0 R\n"
             << "     /Resources " << (xref.size()+1) << " 0 R\n"
             << "     /MediaBox [0 0 612 792]\n"
             << "     /Contents " << (xref.size()+3) << " 0 R\n"
             << "  >>\n"
             << "endobj\n\n";
      

      
      //---
      // This will remap input to eight bit if not already and make input one or
      // three band depending on number of bands.
      //---
      setupInputChain();
      
      switch ( getImageType() )
      {
         case ossimPdfWriter::PNG:
         {
            status = writePngImage( str, xref );
            break;
         }
         case ossimPdfWriter::RAW:
         default:
         {
            status = writeRawImage( str, xref );
            break;
         }
      }
      
   } // Matches: if ( str )
   
   return status;
   
} // End: ossimPdfWriter::writeImageDictionary
   
bool ossimPdfWriter::writePngImage( std::ostream* str, std::vector<std::streamoff>& xref  )
{
   bool status = false;
   if ( str && theInputConnection.valid() && !theAreaOfInterest.hasNans() )
   {
      // See if the ossimPngWriter plugins is there:
      ossimRefPtr<ossimImageFileWriter> writer = ossimImageWriterFactoryRegistry::instance()->
         createWriter( ossimString("ossimPngWriter" ));

      if ( writer.valid() )
      {
         // Things needed.
         ossimRefPtr<ossimImageGeometry> geom = theInputConnection->getImageGeometry();
         const ossimScalarType SCALAR = theInputConnection->getOutputScalarType();
         const ossim_uint32 BANDS = theInputConnection->getNumberOfOutputBands();
         const ossim_uint32 BITS_PER_PIXEL = ossim::getBitsPerPixel( SCALAR );
         const ossim_uint32 SIZE_IN_BYTES =
            theAreaOfInterest.area() * BANDS * ossim::scalarSizeInBytes( SCALAR );
         
         // Connect it to our input.
         writer->connectMyInputTo( theInputConnection->getInput(0) );
         
         // This defaults to on in the general raster writer.  Turn it off.
         writer->setWriteExternalGeometryFlag( false );
         
         // Interleave:
         writer->setOutputImageType(OSSIM_GENERAL_RASTER_BIP);
         
         // Set the area of interest.
         writer->setAreaOfInterest( theAreaOfInterest );
         
         writer->initialize();
         
         if ( traceLog() )
         {
            ossimKeywordlist logKwl;
            writer->saveStateOfAllInputs(logKwl);
            
            ossimFilename logFile = getFilename();
            logFile.setExtension("log");
            ossimKeywordlist kwl;
            writer->saveStateOfAllInputs(kwl);
            kwl.write(logFile.c_str() );
         }
         
         // Resource dictionary:
         
         // Capture position:
         xref.push_back( (std::streamoff)str->tellp() );
         
         *str << xref.size() << " 0 obj % Resource dictionary\n"
                << "  << /ProcSet [/PDF " << (BANDS==1?"/ImageB":"ImageC") << "]\n"
                << "     /XObject << /Im1 " << (xref.size()+1) << " 0 R >>\n"
                << "  >>\n"
                << "endobj\n\n";
         
         // Capture position:
         xref.push_back( (std::streamoff)str->tellp() );
         
         *str << xref.size() << " 0 obj %Image XObject\n"
              << "  << /Type /XObject\n"
              << "     /Subtype /Image\n"
              << "     /Name Im1\n"
              << "     /Width " << theAreaOfInterest.width() << "\n"
              << "     /Height " << theAreaOfInterest.height() << "\n"
              << "     /ColorSpace " << (BANDS==1?"/DeviceGray":"/DeviceRGB") << "\n"
              << "     /BitsPerComponent " << BITS_PER_PIXEL << "\n"
              << "     /Filter /FlateDecode\n";
         
         // If image is small enough, write to memory so the length can be placed up front.
         if ( SIZE_IN_BYTES < MAX_MEM_WRITE )
         {
            std::ostringstream pngStr;
            
            // Set the stream to write to:
            writer->setOutputStream( pngStr );
            
            // Write the image to memory:
            status = writer->writeStream();

            // Add newline to stream per spec.
            pngStr << "\n";
            
            // Add one to length for trailing '\n' as per spec.
            *str << "     /Length " << pngStr.str().size() << "\n"
                 << "  >>\n"
                 << "stream\n";

            // Write to file from memory.
            str->write( pngStr.str().data(), pngStr.str().size() );

            // Close out stream object.
            *str << "endstream\n"
                 << "endobj\n\n";
         }
         else // Write to file placing length in separate object after known.
         {
            // Set the stream to write to:
            writer->setOutputStream( *str );
            
            // Use indirect reference for length.
            *str << "     /Length " << (xref.size()+1) << " 0 R\n"
              << "  >>\n"
              << "stream\n";

            // Grab start offset for length.
            std::streamoff startOffset = str->tellp();
         
            // Write the image to the file:
            status = writer->writeStream();
         
            // Add newline to stream per spec.
            *str << "\n";

            // Grab the end offset for length.
            std::streamoff endOffset = str->tellp();
            
            *str << "endstream\n"
                 << "endobj\n\n";

            // Write the object with length.
            
            // Capture position:
            xref.push_back( (std::streamoff)str->tellp() );
            *str << xref.size() << " 0 obj\n"
                 << "  " << (endOffset-startOffset) << "\n"
                 << "endobj\n\n";
         }
         
         // Capture position:
         xref.push_back( (std::streamoff)str->tellp() );
         
         std::ostringstream os;
         os << "  q\n"                      // Save graphics state\n"
            << "    512 0 0 512 50 50 cm\n" // Translate to 50,50 scale by 512\n"
            << "    /Im1 Do\n"              // Paint image
            << "  Q\n";
         
         *str << xref.size() << " 0 obj\n"
              << "  << /Length " << os.str().size() << " >>\n"
              << "stream\n";
         str->write( os.str().data(), os.str().size() );
         *str << "endstream\n"
              << "endobj\n\n";

      } // Matches: if ( writer.valid() ){
      
   } // Matches: if ( str ...
   
   return status;
   
} // End: ossimPdfWriter::writePngImage

bool ossimPdfWriter::writeRawImage( std::ostream* str,
                                    std::vector<std::streamoff>& xref )
{
   bool status = false;
   if ( str && theInputConnection.valid() && !theAreaOfInterest.hasNans() )
   {
      // Things we need.
      ossimRefPtr<ossimImageGeometry> geom = theInputConnection->getImageGeometry();
      const ossimScalarType SCALAR = theInputConnection->getOutputScalarType();
      const ossim_uint32 BANDS = theInputConnection->getNumberOfOutputBands();
      const ossim_uint32 BITS_PER_PIXEL = ossim::getBitsPerPixel( SCALAR );
      const ossim_uint32 SIZE_IN_BYTES =
         theAreaOfInterest.area() * BANDS * ossim::scalarSizeInBytes( SCALAR );
      
      // Writer:
      ossimRefPtr<ossimImageFileWriter> writer = new ossimGeneralRasterWriter();
      
      // Connect it to our input.
      writer->connectMyInputTo( theInputConnection->getInput(0) );

      // This defaults to on in the general raster writer.  Turn it off.
      writer->setWriteExternalGeometryFlag( false );

      // Set the stream to write to:
      writer->setOutputStream( *str );

      // Interleave:
      writer->setOutputImageType(OSSIM_GENERAL_RASTER_BIP);
      
      // Set the area of interest.
      writer->setAreaOfInterest( theAreaOfInterest );

      writer->initialize();

      if ( traceLog() )
      {
         ossimKeywordlist logKwl;
         writer->saveStateOfAllInputs(logKwl);
         
         ossimFilename logFile = getFilename();
         logFile.setExtension("log");
         ossimKeywordlist kwl;
         writer->saveStateOfAllInputs(kwl);
         kwl.write(logFile.c_str() );
      }

      // Resource dictionary:

      // Capture position:
      xref.push_back( (std::streamoff)str->tellp() );

      *str << xref.size() << " 0 obj % Resource dictionary\n"
             << "  << /ProcSet [/PDF " << (BANDS==1?"/ImageB":"ImageC") << "]\n"
             << "     /XObject << /Im1 " << (xref.size()+1) << " 0 R >>\n"
             << "  >>\n"
             << "endobj\n\n";

      // Capture position:
      xref.push_back( (std::streamoff)str->tellp() );
      
      *str << xref.size() << " 0 obj %Image XObject\n"
           << "  << /Type /XObject\n"
           << "     /Subtype /Image\n"
           << "     /Name Im1\n"
           << "     /Width " << theAreaOfInterest.width() << "\n"
           << "     /Height " << theAreaOfInterest.height() << "\n"
           << "     /ColorSpace " << (BANDS==1?"/DeviceGray":"/DeviceRGB") << "\n"
           << "     /BitsPerComponent " << BITS_PER_PIXEL << "\n";

      // Add one to length for trailing '\n' as per spec.
      *str << "     /Length " << (SIZE_IN_BYTES+1) << "\n"
          << "  >>\n"
          << "stream\n";

      // Write the image to the file:
      status = writer->writeStream();

      // Add newline to stream per spec.
      *str << "\n"
           << "endstream\n"
           << "endobj\n\n";

      // Capture position:
      xref.push_back( (std::streamoff)str->tellp() );
      
      std::ostringstream os;
      os << "  q\n"                      // Save graphics state\n"
         << "    512 0 0 512 50 50 cm\n" // Translate to 50,50 scale by 512\n"
         << "    /Im1 Do\n"              // Paint image
         << "  Q\n";

      *str << xref.size() << " 0 obj\n"
           << "  << /Length " << os.str().size() << " >>\n"
           << "stream\n";
      str->write( os.str().data(), os.str().size() );
      *str << "endstream\n"
           << "endobj\n\n";
      
   } // Matches: if ( str ...
   
   return status;
   
} // End: ossimPdfWriter::writeRawImage

void ossimPdfWriter::writeXref( std::ostream* str,
                                const std::vector<std::streamoff>& xref )
{
   if ( str )
   {
      *str << "xref\n"
           << "0 " << (xref.size()+1) << "\n"
           << "0000000000 65535 f\n";

      std::vector<std::streamoff>::const_iterator i = xref.begin();
      while ( i != xref.end() )
      {
         std::ostringstream xs;
         xs << std::setiosflags(std::ios_base::fixed|std::ios_base::right)
            << std::setfill('0')
            << std::setw(10)
            << std::setfill('0')
            << std::setw(10)
            << (*i)
            << " 00000 n\n";
         str->write( xs.str().data(), xs.str().size() );
         ++i;
      }
      
      *str << "\n";
   }
}

void ossimPdfWriter::writeTrailer( std::ostream* str,
                                   ossim_uint32 entrySize,
                                   std::streamoff xrefOffset )
{
   if ( str )
   {
      *str << "trailer\n"
           << "   << /Size " << entrySize << "\n"
           << "      /Root 1 0 R\n"
           << "   >>\n"
           << "startxref\n"
           << xrefOffset << "\n"
           << "%%EOF"; 
   }
}

bool ossimPdfWriter::saveState(ossimKeywordlist& kwl,
                               const char* prefix)const
{
   return ossimImageFileWriter::saveState(kwl, prefix);
}

bool ossimPdfWriter::loadState(const ossimKeywordlist& kwl,
                               const char* prefix)
{
   return ossimImageFileWriter::loadState(kwl, prefix);
}

bool ossimPdfWriter::isOpen() const
{
   bool result = false;
   if (m_str)
   {
      const std::ofstream* fs = dynamic_cast<const std::ofstream*>(m_str);
      if ( fs )
      {
         result = fs->is_open();
      }
      else
      {
         // Pointer good enough...
         result = true;
      }
   }
   return result;
}


bool ossimPdfWriter::open()
{
   bool result = false;
   
   close();

   // Check for empty filenames.
   if (theFilename.size())
   {
      // ossimOFStream* os = new ossimOFStream();
      std::ofstream* os = new std::ofstream();
      os->open(theFilename.c_str(), ios::out | ios::binary);
      if(os->is_open())
      {
         m_str = os;
         m_ownsStream = true;
         result = true;
      }
      else
      {
         delete os;
         os = 0;
      }
   }
   return result;
}

void ossimPdfWriter::close()
{
   if (m_str)      
   {
      m_str->flush();

      if (m_ownsStream)
      {
         delete m_str;
         m_str = 0;
         m_ownsStream = false;
      }
   }
}

void ossimPdfWriter::getImageTypeList(std::vector<ossimString>& imageTypeList)const
{
   imageTypeList.push_back(ossimString("ossim_pdf"));
}

ossimString ossimPdfWriter::getExtension() const
{
   return ossimString("pdf");
}

bool ossimPdfWriter::hasImageType(const ossimString& imageType) const
{
   if ( (imageType == "ossim_pdf") || (imageType == "application/pdf") )
   {
      return true;
   }

   return false;
}

void ossimPdfWriter::setProperty(ossimRefPtr<ossimProperty> property)
{
   if ( property.valid() )
   {
      std::string name = property->getName().string();
      if ( name == ossimKeywordNames::IMAGE_TYPE_KW )
      {
         addOption( name, property->valueToString().string() ) ;
      }
      return;
   }

   ossimImageFileWriter::setProperty(property);
}

ossimRefPtr<ossimProperty> ossimPdfWriter::getProperty(const ossimString& name)const
{
   ossimRefPtr<ossimProperty> prop = 0;
   
   if ( name == ossimKeywordNames::IMAGE_TYPE_KW )
   {
      
      
      ossimString type;
      getImageType( type.string() );
      
      ossimRefPtr<ossimStringProperty> stringProp =
         new ossimStringProperty( name,
                                  type,
                                  false); //  editable flag

      // Alway support raw (general raster).
      stringProp->addConstraint( ossimString("raw") );

      // Png writer is a plugin so check to see if loaded.
      ossimRefPtr<ossimImageFileWriter> writer = ossimImageWriterFactoryRegistry::instance()->
         createWriter( ossimString("ossimPngWriter" ));
      if ( writer.valid() )     
      {
         stringProp->addConstraint( ossimString("png") );
      }
      prop = stringProp.get();
   }
   else
   {
      prop = ossimImageFileWriter::getProperty(name);
   }
   
   return prop;
}

void ossimPdfWriter::getPropertyNames(std::vector<ossimString>& propertyNames)const
{
   propertyNames.push_back( ossimString( ossimKeywordNames::IMAGE_TYPE_KW ) );
   ossimImageFileWriter::getPropertyNames(propertyNames);
}

bool ossimPdfWriter::setOutputStream(std::ostream& stream)
{
   if (m_ownsStream && m_str)
   {
      delete m_str;
   }
   m_str = &stream;
   m_ownsStream = false;
   return true;
}

void ossimPdfWriter::setupInputChain()
{
   if ( theInputConnection.get() )
   {
      // Capture the input to the image source sequencer(theInputConnection).
      m_savedInput = theInputConnection->getInput( 0 );
      
      ossimScalarType inputScalar = theInputConnection->getOutputScalarType();
      ossim_uint32 bands          = theInputConnection->getNumberOfOutputBands();

      if( inputScalar != OSSIM_UINT8 )
      {
         // Make eight bit.
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_WARN)
               << "ossimPdfWriter::setupInputChain WARNING:"
               << "\nData is being scaled to 8 bit!"
               << "\nOriginal scalar type:  "
               << ossimScalarTypeLut::instance()->
               getEntryString(inputScalar).c_str()
               << std::endl;
         }
         
         //---
         // Attach a scalar remapper to the end of the input chain.  This will
         // need to be unattached and deleted at the end of this.
         //---
         ossimRefPtr<ossimScalarRemapper> sr = new ossimScalarRemapper;

         // Connect remapper's input to sequencer input.
         sr->connectMyInputTo( 0, theInputConnection->getInput(0) );

         // Connet sequencer to remapper.
         theInputConnection->connectMyInputTo( sr.get() );

         // Initialize connections.
         theInputConnection->initialize();
      }

      // Must be one or three band. Note bands are zero based...      
      if ( ( bands != 1 ) && ( bands != 3 ) )
      {
         std::vector<ossim_uint32> bandList;
         
         // Always have one band.
         bandList.push_back( 0 );

         if ( bands > 3 )
         {
            // Use the first three bands.
            bandList.push_back( 1 );
            bandList.push_back( 2 );
         }
         
         ossimRefPtr<ossimBandSelector> bs = new ossimBandSelector();

         // Set the the band selector list.
         bs->setOutputBandList( bandList );
         
         // Connect band selector's input to sequencer input.
         bs->connectMyInputTo( 0, theInputConnection->getInput(0) );

         // Connet sequencer to band selector.
         theInputConnection->connectMyInputTo( bs.get() );

         // Initialize connections.
         theInputConnection->initialize();
      }
      
   } // Matches: if ( theInputConnection )

} // End: void ossimPdfWriter::setupInputChain()

ossimPdfWriter::ossimPdfImageType ossimPdfWriter::getImageType() const
{
   ossimPdfWriter::ossimPdfImageType result = ossimPdfWriter::RAW;

   // Get the type and downcase.
   ossimString os;
   getImageType( os.string() );
   os.downcase();

   if ( os == "png" )
   {
     result = ossimPdfWriter::PNG;
   }
   else if ( os == "raw" )
   {
      result = ossimPdfWriter::RAW;
   }
   else
   {
      // Unknown value:
      ossimNotify(ossimNotifyLevel_WARN)
         << "ossimPdfWriter::getImageCompression WARN\n"
         << "Unhandled image compression type: " << os << std::endl;
   }
   return result;
}

void ossimPdfWriter::getImageType( std::string& type ) const
{
   type = m_kwl->findKey( std::string(ossimKeywordNames::IMAGE_TYPE_KW) );
   if ( type.empty() )
   {
      type = "raw";
   }
}

void ossimPdfWriter::addOption(  const std::string& key, const std::string& value )
{
   m_mutex.lock();
   if ( m_kwl.valid() )
   {
      if ( key.size() && value.size() )
      {
         m_kwl->addPair( key, value );
      }
   }
   m_mutex.unlock();
}
