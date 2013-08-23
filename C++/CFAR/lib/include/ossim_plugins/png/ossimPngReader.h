//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: OSSIM Portable Network Graphics (PNG) reader (tile source).
//
//----------------------------------------------------------------------------
// $Id: ossimPngReader.h 19878 2011-07-28 16:27:26Z dburken $
#ifndef ossimPngReader_HEADER
#define ossimPngReader_HEADER

#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimAppFixedTileCache.h>
#include <png.h>
#include <vector>

class ossimImageData;

class ossimPngReader : public ossimImageHandler
{
public:

   enum ossimPngReadMode
   {
      ossimPngReadUnknown = 0,
      ossimPngRead8       = 1,
      ossimPngRead16      = 2,
      ossimPngRead8a      = 3,
      ossimPngRead16a     = 4 
   };

   /** default constructor */
   ossimPngReader();

   /** virtual destructor */
   virtual ~ossimPngReader();

   /** @return "png" */
   virtual ossimString getShortName() const;

   /** @return "ossim png" */
   virtual ossimString getLongName()  const;

   /** @return "ossimPngReader" */
   virtual ossimString getClassName()    const;

   /**
    *  Returns a pointer to a tile given an origin representing the upper
    *  left corner of the tile to grab from the image.
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
     *  Returns the number of bands in the image.
     *  Satisfies pure virtual from ImageHandler class.
     */
   virtual ossim_uint32 getNumberOfInputBands() const;

   /**
    * Returns the number of bands in a tile returned from this TileSource.
    * Note: we are supporting sources that can have multiple data objects.
    * If you want to know the scalar type of an object you can pass in the
    */
   virtual ossim_uint32 getNumberOfOutputBands()const;

   /**
     *  Returns the number of lines in the image.
     *  Satisfies pure virtual from ImageHandler class.
     */
   virtual ossim_uint32 getNumberOfLines(ossim_uint32 reduced_res_level = 0) const;

   /**
    *  Returns the number of samples in the image.
    *  Satisfies pure virtual from ImageHandler class.
    */
   virtual ossim_uint32 getNumberOfSamples(ossim_uint32 reduced_res_level = 0) const;

   /**
    * Returns the zero based image rectangle for the reduced resolution data
    * set (rrds) passed in.  Note that rrds 0 is the highest resolution rrds.
    */
   virtual ossimIrect getImageRectangle(ossim_uint32 reduced_res_level = 0) const;

   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
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
    * Returns the output pixel type of the tile source.
    */
   virtual ossimScalarType getOutputScalarType() const;

   /**
    * Returns the width of the output tile.
    */
   virtual ossim_uint32    getTileWidth() const;

   /**
    * Returns the height of the output tile.
    */
   virtual ossim_uint32    getTileHeight() const;

   /**
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the ossimImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual ossim_uint32 getImageTileWidth() const;

   /**
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the ossimImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual ossim_uint32 getImageTileHeight() const;

   bool isOpen()const;

   virtual double getMaxPixelValue(ossim_uint32 band = 0)const;

   /** Close method. */
   virtual void close();

protected:
   
   bool readPngInit();
   void readPngVersionInfo();
   ossimString getPngColorTypeString() const;

   /**
    * @Sets the max pixel value.  Attempts to get the sBIT chunk.
    */
   void setMaxPixelValue();

   /**
    *  @brief open method.
    *  @return true on success, false on error.
    */
   virtual bool open();

   /**
    */ 
   void allocate();
   void destroy();

   /**
    * @brief Method to restart reading from the beginning (for backing up).
    * This is needed as libpng requires sequential read from start of the
    * image.
    */
   void restart();

   /**
    * @note this method assumes that setImageRectangle has been called on
    * theTile.
    */
   void fillTile(const ossimIrect& clip_rect, ossimImageData* tile);

   template <class T> void copyLines(T dummy,  ossim_uint32 stopLine);
   template <class T> void copyLinesWithAlpha(T, ossim_uint32 stopLine);

   ossimRefPtr<ossimImageData>  theTile;
   ossimRefPtr<ossimImageData>  theCacheTile;
   ossim_uint8*                 theLineBuffer;
   ossim_uint32                 theLineBufferSizeInBytes;
   FILE*                        theFilePtr;
   ossimIrect                   theBufferRect;
   ossimIrect                   theImageRect;
   ossim_uint32                 theNumberOfInputBands;
   ossim_uint32                 theNumberOfOutputBands;
   ossim_uint32                 theBytePerPixelPerBand;
   ossimIpt                     theCacheSize;


   ossimAppFixedTileCache::ossimAppFixedCacheId theCacheId;

   png_structp      thePngPtr;
   png_infop        theInfoPtr;
   ossim_int8       thePngColorType;
   ossim_uint32     theCurrentRow; // 0 at start or first line
   ossimScalarType  theOutputScalarType;
   ossim_int32      theInterlacePasses;
   ossim_int8       theBitDepth;
   ossimPngReadMode theReadMode;

   std::vector<ossim_float64> theMaxPixelValue;

   bool             theSwapFlag;
   
TYPE_DATA
};

#endif
