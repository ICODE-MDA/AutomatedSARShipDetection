//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:  Class declaration for NITF reader for j2k images using
// OpenJPEG library.
//
// $Id$
//----------------------------------------------------------------------------
#ifndef ossimOpenJpegNitfReader_HEADER
#define ossimOpenJpegNitfReader_HEADER

#include <ossimPluginConstants.h>
#include <ossim/imaging/ossimNitfTileSource.h>
// #include <ossimJ2kSizRecord.h>
// #include <ossimJ2kSotRecord.h>

class OSSIM_PLUGINS_DLL ossimOpenJpegNitfReader : public ossimNitfTileSource
{
public:

   /** default construtor */
   ossimOpenJpegNitfReader();
   
   /** virtural destructor */
   virtual ~ossimOpenJpegNitfReader();

protected:

   /**
    * @param hdr Pointer to image header.
    * @return true if reader can uncompress nitf.
    * */
   virtual bool canUncompress(const ossimNitfImageHeader* hdr) const;

   /**
    * Initializes the data member "theReadMode" from the current entry.
    */
   virtual void initializeReadMode();

   /**
    * Initializes the data member theCompressedBuf.
    */
   virtual void initializeCompressedBuf();

   /**
    * @brief scans the file storing in offsets in "theNitfBlockOffset" and
    * block sizes in "theNitfBlockSize".
    * @return true on success, false on error.  This checks for arrays being
    * the same size as number of blocks.
    */
   virtual bool scanForJpegBlockOffsets();

   /**
    * @brief Uncompresses a jpeg block using the jpeg-6b library.
    * @param x sample location in image space.
    * @param y line location in image space.
    * @return true on success, false on error.
    */
   virtual bool uncompressJpegBlock(ossim_uint32 x, ossim_uint32 y);

private:

TYPE_DATA   
};

#endif /* #ifndef ossimOpenJpegNitfReader_HEADER */
