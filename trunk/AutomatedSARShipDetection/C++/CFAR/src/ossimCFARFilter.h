#ifndef ossimCFARFilter_HEADER
#define ossimCFARFilter_HEADER

#include "ossim/plugin/ossimSharedObjectBridge.h"
#include "ossim/base/ossimString.h"
#include "ossim/imaging/ossimImageSourceFilter.h"

#include <stdlib.h>

#include "opencv/cv.h"
#include "opencv/highgui.h"

class ossimCFARFilter : public ossimImageSourceFilter
{

public:
   ossimCFARFilter(ossimObject* owner=NULL);
   ossimCFARFilter(ossimImageSource* inputSource);
   virtual ~ossimCFARFilter();
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

   double getThreshold(void){return thresholdValue;};
   void setThreshold(double val){thresholdValue = val;};
    
   int getGuardSize(void){return guardSize;};
   void setGuardSize(int val){guardSize = val;};
   
   int getNeighbourSize(void){return neighbourSize;};
   void setNeighbourSize(int val){neighbourSize = val;};

   int getCFARMethod(void){return cfarMethod;};
   void setCFARMethod(int val){cfarMethod = val;};

   void simpleCFAR(cv::Mat& inputImage, cv::Mat& outputImage);
   
   
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
   double thresholdValue;
   int guardSize;
   int neighbourSize;
   int cfarMethod;
TYPE_DATA
};

#endif
