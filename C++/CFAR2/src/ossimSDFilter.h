#ifndef ossimSDFilter_HEADER
#define ossimSDFilter_HEADER

#include "ossim/plugin/ossimSharedObjectBridge.h"
#include "ossim/base/ossimString.h"
#include "ossim/imaging/ossimImageSourceFilter.h"

#include <stdlib.h>
#include <vector>
#include <numeric>
#include <algorithm> 
#include <math.h>
#include <iomanip>

#include "opencv/cv.h"
#include "opencv/highgui.h"

class ossimSDFilter : public ossimImageSourceFilter
{

public:
   ossimSDFilter(ossimObject* owner=NULL);
   ossimSDFilter(ossimImageSource* inputSource);
   virtual ~ossimSDFilter();
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

   int getSpacing(void){return spacing;};
   void setSpacing(int val){spacing = val;};

   double getBandwidth(void){return bw;};
   void setBandwidth(double val){bw = val;};
   
   double getDescendRate(void){return descendRate;};
   void setDescendRate(double val){descendRate = val;};

   int getMaxIterations(void){return iterMax;};
   void setMaxIterations(int val){iterMax = val;};
   
   int getSDType(void){return sdType;};
   void setSDType(int val){sdType = val;};

   void simpleSD(cv::Mat& inputImage, cv::Mat& outputImage);
   
   void findBlobsCC(cv::Mat &binaryImage, 
                std::vector < std::vector<cv::Point2i> > &blobs, 
                std::vector<cv::Point2i> &blobCentres);
   void findBlobsMS(cv::Mat &binaryImage, 
                std::vector < std::vector<cv::Point2i> > &blobs, 
                std::vector<cv::Point2i> &blobCentres);
   
   int meanvector(std::vector<double> &x, std::vector<double> &data, int rows, int cols, double bw2, std::vector<double> &mean);
   double dist(std::vector<double> &A, std::vector<double> &B);
   void meanshift(std::vector<double> &data, int rows, int cols, double bw, double rate, 
                        int iterMax, std::vector<double> &labelledClusters, std::vector<double> &meansFinal);
   void convertBinaryTo8BitBinary(cv::Mat &binaryImage);
      
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
   
   //Helper functions
   void paintCentres(cv::Mat &binaryImage, std::vector<cv::Point2i> &blobCentres);
   void paintBlobs(cv::Mat &binaryImage, std::vector < std::vector<cv::Point2i> > &blobs);
   void find(const cv::Mat& binary, std::vector<cv::Point> &idx);
   
   int scaleValue;
   int sdType;
   int spacing;
   double bw;
   double descendRate;
   int iterMax;
TYPE_DATA
};

#endif
