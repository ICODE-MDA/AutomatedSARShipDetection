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

#ifndef ossimPdfWriter_HEADER
#define ossimPdfWriter_HEADER 1

#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimRtti.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimNBandLutDataObject.h>
#include <ossim/imaging/ossimImageSource.h>
#include <OpenThreads/Mutex>
#include <iosfwd>

/**
 * @class ossimPdfWriter
 *
 * Most of this was coded from the Adobe PDF Reference sixth edition
 * Version 1.7 November 2006 (pdf_reference_1-7.pdf).
 *
 */
class ossimPdfWriter : public ossimImageFileWriter
{
public:

   enum ossimPdfImageType
   {
      UNKNOWN = 0,
      RAW     = 1, // general raster band interleaved by pixel
      PNG     = 2  // Requires ossim_plugin png plugin.
   };

   // Annonymous enus:
   enum
   {
      MAX_MEM_WRITE = 33554432 // 1024*1024*32 (32MB) 
   };

   /* default constructor */
   ossimPdfWriter();

   /* virtual destructor */
   virtual ~ossimPdfWriter();

   /** @return "pdf writer" */
   virtual ossimString getShortName() const;

   /** @return "ossim pdf writer" */
   virtual ossimString getLongName()  const;

   /** @return "ossimPdfReader" */
   virtual ossimString getClassName()    const;

   /**
    * Returns a 3-letter extension from the image type descriptor 
    * (theOutputImageType) that can be used for image file extensions.
    *
    * @param imageType string representing image type.
    *
    * @return the 3-letter string extension.
    */
   virtual ossimString getExtension() const;

   /**
    * void getImageTypeList(std::vector<ossimString>& imageTypeList)const
    *
    * Appends this writer image types to list "imageTypeList".
    *
    * This writer only has one type "pdf".
    *
    * @param imageTypeList stl::vector<ossimString> list to append to.
    */
   virtual void getImageTypeList(std::vector<ossimString>& imageTypeList)const;
   
   virtual bool isOpen()const;   
   
   virtual bool open();

   virtual void close();
   
   /**
    * saves the state of the object.
    */
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * Will set the property whose name matches the argument
    * "property->getName()".
    *
    * @param property Object containing property to set.
    */
   virtual void setProperty(ossimRefPtr<ossimProperty> property);

   /**
    * @param name Name of property to return.
    * 
    * @returns A pointer to a property object which matches "name".
    */
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const;

   /**
    * Pushes this's names onto the list of property names.
    *
    * @param propertyNames array to add this's property names to.
    */
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames)const;

   bool hasImageType(const ossimString& imageType) const;

   /**
    * @brief Method to write the image to a stream.
    *
    * @return true on success, false on error.
    */
   virtual bool writeStream();

   /**
    * @brief Sets the output stream to write to.
    *
    * The stream will not be closed/deleted by this object.
    *
    * @param output The stream to write to.
    */
   virtual bool setOutputStream( std::ostream& stream );

private:

   /**
    * @brief Writes the file to disk or a stream.
    * @return true on success, false on error.
    */
   virtual bool writeFile();

   /**
    * @brief Writes header.
    * @param str Stream to write to.
    */
   void writeHeader( std::ostream* str );
   
   /**
    * @brief Writes Catalog object.
    * @param str Stream to write to.
    */
   void writeCatalog( std::ostream* str );

   /**
    * @brief Writes Pages object.
    * @param str Stream to write to.
    * @objectNumber 
    */
   void writePages( std::ostream* str,
                    ossim_uint32 objectNumber,
                    ossim_uint32 count );

   /**
    * Writes the image dictionary.
    * @param str Stream to write to.
    * @return true on success, false on error.
    */
   bool writeImage( std::ostream* str,
                    std::vector<std::streamoff>& xref );

   /**
    * Writes the image dictionary using png image format.
    * @param str Stream to write to.
    * @param xref To capture object positions.
    * @return true on success, false on error.
    */
   bool writePngImage( std::ostream* str,
                       std::vector<std::streamoff>& xref );

   /**
    * Writes the image dictionary using raw general raster format.
    * @param str Stream to write to.
    * @param xref To capture object positions.
    * @return true on success, false on error.
    */
   bool writeRawImage( std::ostream* str,
                       std::vector<std::streamoff>& xref );


   /**
    * @brief Writes Cross Reference(xref) section.
    * @param str Stream to write to.
    * @param xref Array of object offsets.
    */
   void writeXref( std::ostream* str,
                   const std::vector<std::streamoff>& xref );

   /**
    * @brief Writes trailer.
    * @param str Stream to write to.
    * @param entrySize Number of entries, one plus object count.
    * @param xrefOffset Offset to cross reference(xref) table. 
    */
   void writeTrailer( std::ostream* str,
                      ossim_uint32 entrySize,
                      std::streamoff xrefOffset );
   
   /**
    * @brief Sets up input image source.
    * This sets up the chain fed to theInputConnection which is an image
    * source sequencer.  This will remap input to eight bit if not already and
    * make input one or three band depending on number of bands. Also sets
    * m_saveInput for reconnection at the end of write.
    */
   void setupInputChain();

   /**
    * @brief Gets the enumerated value of IMAGE_TYPE_KW lookup.
    *
    * This is the image type of the stream embedded in the pdf.
    * Default "raw" if not found.
    * 
    * @return ossimPdfImageType, e.g. PNG, RAW...
    */
   ossimPdfImageType getImageType() const;   

   /**
    * @brief Initializes image type from IMAGE_TYPE_KW lookup.
    *
    * This is the image type of the stream embedded in the pdf.
    * Default "raw" if not found.
    *
    * @param type Initialized by this.
    */
   void getImageType( std::string& type ) const;

   /**
    * @brief Adds option to m_kwl with mutex lock.
    * @param key
    * @param value
    */
   void addOption( const std::string& key, const std::string& value );

   std::ostream* m_str;
   bool m_ownsStream;

   /**
    * Holds the origin end of the chain connected to theInputConnection.
    */
   ossimRefPtr<ossimConnectableObject> m_savedInput;

   /** Holds all options in key, value pair map. */
   ossimRefPtr<ossimKeywordlist> m_kwl;

   OpenThreads::Mutex m_mutex;

TYPE_DATA
};

#endif /* #ifndef ossimPdfWriter_HEADER */
