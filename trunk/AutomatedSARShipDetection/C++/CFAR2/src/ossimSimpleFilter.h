#ifndef ossimSimpleFilter_HEADER
#define ossimSimpleFilter_HEADER

#include "ossim/plugin/ossimSharedObjectBridge.h"
#include "ossim/base/ossimString.h"
#include "ossim/imaging/ossimImageSourceFilter.h"

#include <stdlib.h>

#include "opencv/cv.h"
#include "opencv/highgui.h"

class ossimSimpleFilter : public ossimImageSourceFilter
{

public:
   ossimSimpleFilter(ossimObject* owner=NULL);
   ossimSimpleFilter(ossimImageSource* inputSource);
   virtual ~ossimSimpleFilter();
   ossimString getShortName()const
      {
         return ossimString("SimpleOssimFilter");
      }
   
   ossimString getLongName()const
      {
         return ossimString("OpenCV Ossim Filter");
      }
   
   virtual ossimRefPtr<ossimImageData> getTile(const ossimIrect& tileRect, ossim_uint32 resLevel=0);
   
   virtual void initialize();
   
   virtual ossimScalarType getOutputScalarType() const;
   
   ossim_uint32 getNumberOfOutputBands() const;
 
   virtual bool saveState(ossimKeywordlist& kwl,
                          const char* prefix=0)const;
   
   int getScaleValue(void){return scaleValue;};
   void setScaleValue(int val){scaleValue = val;};

   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const ossimKeywordlist& kwl,
                          const char* prefix=0);

   /*
   * Methods to expose thresholds for adjustment through the GUI
   */
   virtual void setProperty(ossimRefPtr<ossimProperty> property);
   virtual ossimRefPtr<ossimProperty> getProperty(const ossimString& name)const;
   virtual void getPropertyNames(std::vector<ossimString>& propertyNames)const;

protected:
   ossimRefPtr<ossimImageData> outputTile; // Output tile Output tile
   void runUcharTransformation(ossimImageData* tile); 

   int scaleValue;
TYPE_DATA
};

#endif
