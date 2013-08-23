//----------------------------------------------------------------------------
//
// File: ossimPdfReader.h
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: OSSIM GeoPDF reader.
//
//----------------------------------------------------------------------------
// $Id: ossimPdfReader.h 2679 2011-06-06 16:38:44Z david.burken $
#ifndef ossimPdfReader_HEADER
#define ossimPdfReader_HEADER 1

#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimUnitConversionTool.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/projection/ossimProjection.h>

class ossimPdfReader : public ossimImageHandler
{
public:

   /** default constructor */
   ossimPdfReader();

   /** virtual destructor */
   virtual ~ossimPdfReader();

   /**
    *  @brief open method.
    *
    *  Satisfies ossimImageHandler::open pure virtual.
    *  
    *  @return Returns true on success, false on error.
    *
    *  @note This method relies on the data member ossimImageData::theImageFile
    *  being set.  Callers should do a "setFilename" prior to calling this
    *  method or use the ossimImageHandler::open that takes a file name and an
    *  entry index.
    */
   virtual bool open();

   /**
    *  @brief is open method.
    *
    *  Satisfies ossimImageHandler::isOpen pure virtual.
    *
    *  @return true if open, false if not.
    */
   virtual bool isOpen()const;

   /**
    * @brief Close method.
    */
   virtual void close();

   /**
    *  Returns a pointer to a tile given an origin representing the upper left
    *  corner of the tile to grab from the image.
    *  Satisfies pure virtual from TileSource class.
    */
   virtual ossimRefPtr<ossimImageData> getTile(const  ossimIrect& rect,
                                               ossim_uint32 resLevel=0);
   
   /**
    * Method to get a tile.   
    *
    * @param result The tile to stuff.  Note The requested rectangle in full
    * image space and bands should be set in the result tile prior to
    * passing.  It will be an error if:
    * result.getNumberOfBands() != this->getNumberOfOutputBands()
    *
    * @return true on success false on error.  If return is false, result
    *  is undefined so caller should handle appropriately with makeBlank or
    * whatever.
    */
   virtual bool getTile(ossimImageData* result, ossim_uint32 resLevel=0);
   
   /**
    * @brief Gets bands.
    *
    * Satisfies ossimImageSource::getNumberOfInputBands pure virtual.
    * 
    * @retrun Number of bands.
    */
   virtual ossim_uint32 getNumberOfInputBands() const;

   /**
    * @brief Gets lines.
    *
    * Satisfies ossimImageHandler::getNumberOfLines pure virtual.
    *
    * @param resLevel Reduced resolution level to return lines of.
    * Default = 0
    *
    * @return The number of lines for specified reduced resolution level.
    */
   virtual ossim_uint32 getNumberOfLines(ossim_uint32 resLevel = 0) const;

   /**
    * @brief Gets samples.
    *
    * Satisfies ossimImageHandler::getNumberOfSamples
    *
    * @param resLevel Reduced resolution level to return samples of.
    * Default = 0
    *
    * @return The number of samples for specified reduced resolution level.
    */
   virtual ossim_uint32 getNumberOfSamples(ossim_uint32 resLevel = 0) const;
    
   /**
    * @brief Gets tile width.
    *
    * Satisfies ossimImageHandler::getImageTileWidth pure virtual.
    * 
    * @return The tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the ossimImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual ossim_uint32 getImageTileWidth() const;

   /**
    * @brief Gets tile height.
    *
    * Satisfies ossimImageHandler::getImageTileHeight pure virtual.
    * 
    * @return The tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the ossimImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual ossim_uint32 getImageTileHeight() const;

   /** @return The width of the output tile. */
   virtual ossim_uint32 getTileWidth() const;
   
   /** @returns The height of the output tile. */
   virtual ossim_uint32 getTileHeight() const;

   /** @return The output pixel type of the tile source. */
   ossimScalarType getOutputScalarType() const;

   /**
    * @brief Gets entry list.
    * @param entryList This is the list to initialize with entry indexes.
    */
   virtual void getEntryList(std::vector<ossim_uint32>& entryList) const;

   /** @return The current entry number. */
   virtual ossim_uint32 getCurrentEntry() const;

   /**
    * @param entryIdx Entry number to select.
    * @return true if it was able to set the current entry and false otherwise.
    */
   virtual bool setCurrentEntry(ossim_uint32 entryIdx);

   /** @return "pdf" */
   virtual ossimString getShortName() const;
   
   /** @return "ossim pdf reader" */
   virtual ossimString getLongName()  const;

   /**
    * Returns the image geometry object associated with this tile source or
    * NULL if non defined.  The geometry contains full-to-local image
    * transform as well as projection (image-to-world).
    */
   virtual ossimRefPtr<ossimImageGeometry> getImageGeometry();

   /** @return Min pixel value. */
   virtual double getMinPixelValue(ossim_uint32 band=0) const;

   /** @return Min pixel value. */
   virtual double getMaxPixelValue(ossim_uint32 band=0) const;

   /** @return Min pixel value. */
   virtual double getNullPixelValue(ossim_uint32 band=0) const;

   /**
    * @brief Method to save the state of an object to a keyword list.
    * @return true if ok or false on error.
    */
   virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;
  
   /**
    * @brief Method to the load (recreate) the state of an object from a keyword list.
    * @return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);

   /**
    * @brief Set propterty method. Overrides ossimImageHandler::setProperty.
    *
    * Current property name handled:
    * "scale" One double value representing the scale in meters per pixel. It is
    * assumed the scale is same for x and y direction.
    * 
    * @param property to set.
    */
   virtual void setProperty(ossimRefPtr<ossimProperty> property);

   /**
    * @brief Get propterty method. Overrides ossimImageHandler::getProperty.
    * @param name Property name to get.
    */
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name) const;

   /**
    * @brief Get propterty names. Overrides ossimImageHandler::getPropertyNames.
    * @param propertyNames Array to initialize.
    */
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames) const;

   /** @return The total number of decimation levels. */
   virtual ossim_uint32 getNumberOfDecimationLevels() const;

protected:
   /**
    * @brief Will complete the opening process.
    *
    * Overrides ossimImageHandler::completeOpen() as we do not ever have overviews.
    */
   virtual void completeOpen();
   
private:
  
   /**
    * @brief Checks for %PDF in first four bytes and %%EOF last five bytes.
    * @return true on success, false on error.
    */
   bool checkSignature();

   /**
    * @brief Initializes the cross reference table.
    * @return true on success, false on error.
    */
   bool initXrefTable();

   bool backupLine( ossim_int32 atMostBytes );

   ossimRefPtr<ossimImageData>  m_tile;
   ossim_uint8                  m_entry;
   std::ifstream                m_str;

   // Cross reference tables.  Holds the offset of each object.  Written at the end.
   std::vector<std::streamoff>  m_xref;

   OpenThreads::Mutex           m_mutex;

TYPE_DATA
};

inline bool ossimPdfReader::isOpen()const
{
   return ( m_str.is_open() );
}

#endif /* #ifndef ossimPdfReader_HEADER */
