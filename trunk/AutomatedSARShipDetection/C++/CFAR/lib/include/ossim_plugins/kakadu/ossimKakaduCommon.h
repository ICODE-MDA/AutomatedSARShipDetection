//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Common code for this plugin.
//
//----------------------------------------------------------------------------
// $Id: ossimKakaduCommon.h 19904 2011-08-05 17:50:32Z dburken $

#ifndef ossimKakaduCommon_HEADER
#define ossimKakaduCommon_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <iosfwd>
#include <vector>

class jp2_source;
class kdu_codestream;
class kdu_thread_env;

class ossimImageData;
class ossimIrect;

struct kdu_channel_mapping;
struct kdu_coords;
struct kdu_dims;
struct kdu_thread_queue;

namespace ossim
{
   /**
    * @brief Convenience method to convert ossimIrect to kdu_dims.
    * 
    * @param rect The ossimIrect to convert.
    * 
    * @param dims This kdu_dim to initialize.
    */
   void getDims(const ossimIrect& rect, kdu_dims& dims);
   
   /**
    * @brief Convenience method to convert kdu_dims to ossimIrect.
    * 
    * @param dims This kdu_dim to convert.
    * 
    * @param rect The ossimIrect to initialize.
    */
   void getRect(const kdu_dims& dims, ossimIrect& rect);
   
   /**
    * @brief Sets clipRegion from region, and image dimensions for level.
    *
    * This will clip region to the image dimensions for
    * the given discard_levels.  Returns false
    * if no intersection.
    * 
    * @param codestream Stream to pull from.
    * 
    * @param region The region wanted from codestream.
    * 
    * @param discard_levels The resolution level, 0 being full res.
    * 
    * @param clipRegion The region to initialize.
    * 
    * @return true on success, false if no intersection or there were
    * codestream or pointer issues.
    */
   bool clipRegionToImage(kdu_codestream& codestream,
                          kdu_dims& region,
                          int discard_levels,
                          kdu_dims& clipRegion);
   
   /**
    * @brief Gets image and tile dimensions from codestream for each
    * resolution level (rlevel).  Note that on entry the arrays are cleared
    * so they will be empty on failure.
    * 
    * @param codestream The codestream to read from.
    *
    * @param imageDims Array to initialize with image size of each rlevel.
    * This will have any sub image offset applied to it.
    * 
    * @param tileDims Array to initialize with tile size of each rlevel.
    * This is zero base tile size.  No sub image offset if there is one.
    * 
    * @return true on success, false on error.
    *
    * @note On entry the arrays are cleared so they will can be empty on
    * failure.
    */
   bool getCodestreamDimensions(kdu_codestream& codestream,
                                std::vector<ossimIrect>& imageDims,
                                std::vector<ossimIrect>& tileDims);

   /**
    * @brief Copies region from codestream to tile at a given rlevel.
    *
    * This method takes a channelMapping and decompresses all bands at once.
    * Specifically needed to convert YCC color space to RGB.
    *
    * @param Kakadu channel mapping object.
    * @param codestream Stream to pull from.
    * @param region The region wanted from codestream.
    * @param discard_levels The resolution level, 0 being full res.
    * @param threadEnv Pointer to kdu_thread_env.
    * @param threadQueue Pointer to kdu_thread_queue.
    * @param destTile The ossimImageData object to copy to.
    * @return true on success, false on error.
    */
   bool copyRegionToTile(kdu_channel_mapping* channelMapping,
                         kdu_codestream& codestream,
                         int discard_levels,
                         kdu_thread_env* threadEnv,
                         kdu_thread_queue* threadQueue,
                         ossimImageData* destTile);
   
   /**
    * @brief Copies region from codestream to tile at a given rlevel.
    *
    * This method takes a codestream and decompresses each band separately.
    * Specifically needed for n-band data where the kdu_channel_mapping will
    * not work.
    * 
    * @param codestream Stream to pull from.
    * @param region The region wanted from codestream.
    * @param discard_levels The resolution level, 0 being full res.
    * @param threadEnv Pointer to kdu_thread_env.
    * @param threadQueue Pointer to kdu_thread_queue.
    * @param destTile The ossimImageData object to copy to.
    * @return true on success, false on error.
    */
   bool copyRegionToTile(kdu_codestream& codestream,
                         int discard_levels,
                         kdu_thread_env* threadEnv,
                         kdu_thread_queue* threadQueue,
                         ossimImageData* destTile);

   /**
    * @brief Un-normalizes float tile from kdu_region_decompressor::process method.
    *
    * Takes tile which is assumed to be normalized by kakadu and stretches between the
    * tile min and max.
    */
   void unNormalizeTile(ossimImageData* result);

   /**
    * @brief Computes the number of decimation levels to get to the overview
    * stop dimension.
    *
    * This uses ossim preferences "overview_stop_dimension" value if set else
    * it uses the default tile size to compute the number of levels.
    * 
    * @param rect The rectangle of the first level.
    *
    * @return The number of decimation levels needed.
    */
   ossim_int32 computeLevels(const ossimIrect& rect);

   /** @brief Convenience print method for kdu_codestream */
   std::ostream& print(std::ostream& out, kdu_codestream& cs);

   /** @brief Convenience print method for kdu_dims */
   std::ostream& print(std::ostream& out, const kdu_dims& dims);
   
   /** @brief Convenience print method for kdu_coords */
   std::ostream& print(std::ostream& out, const kdu_coords& coords);

} // matches: namespace ossim

#endif /* matches: #ifndef ossimKakaduCommon_HEADER */
