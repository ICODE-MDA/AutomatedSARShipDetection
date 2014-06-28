// Copyright (C) 2010 Argongra 
//
// OSSIM is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//
// You should have received a copy of the GNU General Public License
// along with this software. If not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-
// 1307, USA.
//
// See the GPL in the COPYING.GPL file for more details.
//
//*************************************************************************

#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimU8ImageData.h>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/imaging/ossimImageSourceFactoryBase.h>
#include <ossim/imaging/ossimImageSourceFactoryRegistry.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimNumericProperty.h>

#include "ossimSDFilter.h"

RTTI_DEF1(ossimSDFilter, "ossimSDFilter", ossimImageSourceFilter)

ossimSDFilter::ossimSDFilter(ossimObject* owner)
   :ossimImageSourceFilter(owner)
{
}

ossimSDFilter::ossimSDFilter(ossimImageSource* inputSource)
   : ossimImageSourceFilter(NULL, inputSource),
     outputTile(NULL)
{
}

ossimSDFilter::~ossimSDFilter()
{
}

ossimRefPtr<ossimImageData> ossimSDFilter::getTile(const ossimIrect& tileRect,
                                                                ossim_uint32 resLevel)
{
  
	if(!isSourceEnabled())
   	{
	      return ossimImageSourceFilter::getTile(tileRect, resLevel);
	}
   
   	if(!outputTile.valid()) initialize();
	if(!outputTile.valid()) return 0;
  
	ossimRefPtr<ossimImageData> data = 0;
	if(theInputConnection)
	{
		data  = theInputConnection->getTile(tileRect, resLevel);
   	} else {
	      return 0;
   	}

	if(!data.valid()) return 0;
	if(data->getDataObjectStatus() == OSSIM_NULL ||  data->getDataObjectStatus() == OSSIM_EMPTY)
   	{
	     return 0;
   	}

	outputTile->setImageRectangle(tileRect);
	outputTile->makeBlank();
   
	outputTile->setOrigin(tileRect.ul());
	runUcharTransformation(data.get());
   
	if(tileRect.ul().x % 1024 == 0 && tileRect.ul().y % 1024 == 0)
       	 std::cout << "Processing tile: (" << tileRect.ul().x << "," << tileRect.ul().y << ")" << std::endl; 
   	
	return outputTile;
   
}

void ossimSDFilter::initialize()
{
  if(theInputConnection)
  {
      ossimImageSourceFilter::initialize();

      outputTile = new ossimU8ImageData(this,
				     theInputConnection->getNumberOfOutputBands(),   
                                     theInputConnection->getTileWidth(),
                                     theInputConnection->getTileHeight());  
      outputTile->initialize();
     
   }

}

ossimScalarType ossimSDFilter::getOutputScalarType() const
{
   if(!isSourceEnabled())
   {
      return ossimImageSourceFilter::getOutputScalarType();
   }
   
   return OSSIM_UCHAR;
}

ossim_uint32 ossimSDFilter::getNumberOfOutputBands() const
{
   if(!isSourceEnabled())
   {
      return ossimImageSourceFilter::getNumberOfOutputBands();
   }
   return theInputConnection->getNumberOfOutputBands();
}

bool ossimSDFilter::saveState(ossimKeywordlist& kwl,  const char* prefix)const
{
   ossimImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix,"aperture_size",0,true);
   
   return true;
}

bool ossimSDFilter::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   ossimImageSourceFilter::loadState(kwl, prefix);

   const char* lookup = kwl.find(prefix, "aperture_size");
   if(lookup)
   {
//       setApertureSize(ossimString(lookup).toInt());
   }
   return true;
}

void ossimSDFilter::runUcharTransformation(ossimImageData* tile) {
		
	// Build up OpenCV image
	int nChannels = tile->getNumberOfBands();
	
	// Run through each channel, get input image, process it then save it to output image
	for(int k=0; k<nChannels; k++) 
	{
	  	// Grab input tile and create shallow OpenCV image (no deep copying)
		ossim_uint16 *inBuf = (ossim_uint16*)tile->getBuf(k);
		cv::Mat inputTile(tile->getHeight(), tile->getWidth(), CV_16UC1, (ossim_uint16*)inBuf);
		
		// Scale input values by some factor (scaleValue)
		cv::divide(inputTile,cv::Scalar::all(scaleValue),inputTile);

		// Convert input into 8 bit, unsigned single channel
		inputTile.convertTo(inputTile,CV_8UC1);
		
		// Threshold image using SD
		cv::Mat inputClone;
		inputClone = inputTile.clone();
		simpleSD(inputClone, inputTile);

		uchar *outBuf = (uchar*)outputTile->getBuf(k);
		cv::Mat outputTile(tile->getHeight(), tile->getWidth(), CV_8UC1, (unsigned char *)outBuf);

		// Write processed data to output
		for (unsigned int i = 0; i < tile->getWidth(); i++)
		{
		  for (unsigned int j = 0; j < tile->getHeight(); j++)
		  {
			outputTile.at<uchar>(j,i) = (uchar)(inputTile.at<uchar>(j,i));
		  }
		}
		
	}

	outputTile->validate(); 
}

void ossimSDFilter::simpleSD(cv::Mat& inputImage, cv::Mat& outputImage)
{
  outputImage = inputImage;
  std::vector < std::vector<cv::Point2i > > blobs;
  std::vector<cv::Point2i> blobCentres;  

  if(sdType == 0)
   findBlobsCC(inputImage, blobs, blobCentres);
  else
   findBlobsMS(inputImage, blobs, blobCentres);
    
   outputImage = cv::Mat(inputImage.rows, inputImage.cols, CV_8UC1, cv::Scalar::all(0));
   paintCentres(outputImage, blobCentres);
   
   //Use LUT to ensure Binary image in true = 255 and false = 0 for visualisation 
   convertBinaryTo8BitBinary(outputImage);
}

void ossimSDFilter::setProperty(ossimRefPtr<ossimProperty> property)

{

        if(!property) return;

        ossimString name = property->getName();



        if(name == "aperture_size")

        {

                
        }

		else

		{

		  ossimImageSourceFilter::setProperty(property);

		}

}



ossimRefPtr<ossimProperty> ossimSDFilter::getProperty(const ossimString& name)const

{

        if(name == "aperture_size")

        {

                ossimNumericProperty* numeric = new ossimNumericProperty(name,

                        ossimString::toString(0),

                        1, 7);

                numeric->setNumericType(ossimNumericProperty::ossimNumericPropertyType_INT);

                numeric->setCacheRefreshBit();

                return numeric;

        }

        return ossimImageSourceFilter::getProperty(name);

}



void ossimSDFilter::getPropertyNames(std::vector<ossimString>& propertyNames)const

{

        ossimImageSourceFilter::getPropertyNames(propertyNames);

        propertyNames.push_back("aperture_size");

}




/*! @brief Converts between different types of binary images
 *
 * OpenCV allows for binary images to be specified in a number of 
 * ways. To ensure visualisation is correct this function converts
 * a binary image that has a high value of "1" to a high value of
 * "255" (maximum for an 8-bit image)
 * 
 * @param binaryImage the input image to be converted (type CV_8UC1)
 */
void ossimSDFilter::convertBinaryTo8BitBinary(cv::Mat& binaryImage)
{
    cv::Mat table(1, 256, CV_8U);
    uchar *p =  table.data;
    p[0] = 0;
    p[1] = 255;
    p[255] = 255;
    cv::LUT(binaryImage, table, binaryImage);
}

/*! @brief Finds the non-zero locations of the true values in the binary image
 *
 * 
 * @param binaryImage the input image to be tested for binary points (type CV_8UC1)
 * @param idx return vector of all the cv::Points that were greater than zero
 */

void ossimSDFilter::find(const cv::Mat& binaryImage, vector< cv::Point >& idx)
{
    assert(binaryImage.cols > 0 && binaryImage.rows > 0 && binaryImage.channels() == 1 && binaryImage.depth() == CV_8U);
    const int M = binaryImage.rows;
    const int N = binaryImage.cols;
    for (int m = 0; m < M; ++m) {
        const uchar* bin_ptr = binaryImage.ptr<uchar>(m);
        for (int n = 0; n < N; ++n) {
            if (bin_ptr[n] > 0) idx.push_back(cv::Point(n,m));
        }
    }
}

/*! @brief Helper function to draw blobs
 *
 * Paints the blobs (connected components) found in the vector on
 * the binary image.
 * 

 * @param binaryImage the input image to be painted on (type CV_8UC1)
 * @param blobs the 2D list of 2D points to be painted
 */

void ossimSDFilter::paintBlobs(cv::Mat& binaryImage, vector< vector< cv::Point2i > >& blobs)
{
  //For each connect component
  for(std::vector< std::vector<cv::Point2i> >::iterator it = blobs.begin(); it != blobs.end(); ++it) 
  {
    //Grab coordinates of points in contour
    std::vector<cv::Point2i> points;
    points = *it;
    
    //Paint each of the coordinates
    paintCentres(binaryImage, points);
  }
}

/*! @brief Helper function to paint a list of points
 * * 
 * @param binaryImage the input image to be painted on (type CV_8UC1)
 * @param blobCentres the pixel coordinates to be painted
 */

void ossimSDFilter::paintCentres(cv::Mat& binaryImage, vector< cv::Point2i >& blobCentres)
{
  //For each connect component draw centroid on binary image
  for(std::vector<cv::Point2i>::iterator it = blobCentres.begin(); it != blobCentres.end(); ++it) 
    binaryImage.at<uchar>(it->y, it->x) = 255;
 

}


/*! @brief Connected Component (CC) and Centroid Finding Function
 * 
 * This function runs through a binary image and finds the blobs
 * centroids using the Connected Component and morphological method
 * 
 * @param binaryImage the binary either (0-255) or (0-1) input image to be CC processed 
 * @param blobs 2D list of 2D points 
 * @param blobCentres list of 2D points representing centroids of each blob
 */
void ossimSDFilter::findBlobsCC(cv::Mat& binaryImage, vector< vector< cv::Point2i > >& blobs, vector< cv::Point2i >& blobCentres)
{
    blobs.clear();
    blobCentres.clear();

    cv::Mat labelImage,binaryImageTemp;
    
    //Perform morphological closing if spacing is not zero
    if(spacing > 0)
     cv::morphologyEx(binaryImage, binaryImage, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(spacing,spacing)));
   
    binaryImage.convertTo(binaryImage, CV_8UC1);
    
    //Ensure binary image is in 0 - 1 format rather than 0 - 255 format used usually used
    //when displaying an image (use a LUT)
    cv::Mat lookup(1, 256, CV_8U);
    uchar *p =  lookup.data;
    p[0] = 0; p[1] = 1; p[255] = 1;
    cv::LUT(binaryImage, lookup, binaryImageTemp);
    binaryImageTemp.convertTo(labelImage, CV_32SC1);

    //Start labeling at 2 because 0 and 1 are already used
    int labelCount = 2;
    
    //Run through image 
    for(int y=0; y < labelImage.rows; y++) {
        int *row = (int*)labelImage.ptr(y);
        for(int x=0; x < labelImage.cols; x++) {
            if(row[x] != 1) {
                continue;
            }

            cv::Rect rect;
            cv::floodFill(labelImage, cv::Point(x,y), labelCount, &rect, 0, 0, 8);
	    
            int iCounter = 0, iVal = 0, jCounter = 0, jVal = 0;
            std::vector <cv::Point2i> blob;

            for(int i=rect.y; i < (rect.y+rect.height); i++) {
                int *row2 = (int*)labelImage.ptr(i);
                for(int j=rect.x; j < (rect.x+rect.width); j++) {
                    if(row2[j] != labelCount) {
                        continue;
                    }

                    blob.push_back(cv::Point2i(j,i));
			
  		    //Update centroid stats
		    iCounter++; jCounter++;
		    iVal += i; jVal += j;  		    

                }
            }

	    //Find centroid (nn, add current label to new 'blob' then update the labelCounter;
	    blobCentres.push_back(cv::Point2i(floor((double(jVal)/double(jCounter))+0.5),
					      floor((double(iVal)/double(iCounter))+0.5)));
	    
            blobs.push_back(blob);

            labelCount++;
        }
    }

}

/*! @brief Mean Shift (MS) and Centroid Finding Function
 * 
 * This function runs through a binary image and finds the blobs
 * centroids using the mean shift method
 * 
 * @param binaryImage the binary either (0-255) or (0-1) input image to be MS processed 
 * @param blobs 2D list of 2D points 
 * @param blobCentres list of 2D points representing centroids of each blob
 */
void ossimSDFilter::findBlobsMS(cv::Mat& binaryImage, vector< vector< cv::Point2i > >& blobs, vector< cv::Point2i >& blobCentres)
{
  //Clear all blobs
  blobs.clear();
  blobCentres.clear();

  //Ensure input image is 8CU1
  binaryImage.convertTo(binaryImage,CV_8UC1);
  
  //Find nonzero coordinates in binary image
  find(binaryImage, blobCentres);
  
  int rows = 2; 
  int cols = blobCentres.size();
  
  std::vector<double> flatBlobs;
  std::vector<double> labelledClusters;
  std::vector<double> meansFinal;
  
  
  flatBlobs.resize(rows*cols);
  labelledClusters.resize(rows*cols);
  
  int counter = 0;
  for(std::vector<cv::Point2i>::iterator it = blobCentres.begin(); it != blobCentres.end(); ++it, counter++) 
  {
   flatBlobs[counter*rows + 0] = it->x;
   flatBlobs[counter*rows + 1] = it->y;
  }
  
  meanshift(flatBlobs, rows, cols, bw, descendRate, iterMax, labelledClusters, meansFinal);

  cv::Mat test = cv::Mat(binaryImage.rows, binaryImage.cols, CV_8UC1, cv::Scalar::all(0));
  
  blobCentres.clear();
  for(int i = 0; i < counter; i++) 
   blobCentres.push_back(cv::Point2i(floor(meansFinal[i*rows]+0.5),floor(meansFinal[i*rows+1]+0.5))); 
  
  paintCentres(test, blobCentres);
  blobCentres.clear();
  find(test, blobCentres);
}

/*! @brief Finds the mean distance vector between data and point x within bandwidth specified by bw
 * 
 */
int ossimSDFilter::meanvector(std::vector< double >& x, std::vector< double >& data, int rows, int cols, double bw2, std::vector< double >& mean)
{
    double distanceToPoint;
    int columnCounter = 0;
    int pointCounter = 0;
    int i = 0, j = 0;
    
    std::fill(mean.begin(), mean.end(), 0.0);
    
    for (i = 0; i < cols; i++) 
    {
        distanceToPoint = 0.0;
	
        j = 0;
	for(std::vector<double>::iterator itMean = mean.begin(); itMean != mean.end(); ++itMean, ++j)      
        {
	  distanceToPoint += (x.at(j)-data.at(columnCounter)) * (x.at(j)-data.at(columnCounter));
	  columnCounter++;
	}
	
        //Use point in mean if distance to x is less than bw squared
	if (distanceToPoint < bw2) 
	{
            columnCounter = columnCounter - rows;
	    
	    j=0;
	    for(std::vector<double>::iterator itMean = mean.begin(); itMean != mean.end(); ++itMean)      
	    {
              *itMean += data.at(columnCounter);
	      columnCounter++;
            }
            pointCounter++;
        }
    }
    
    for(std::vector<double>::iterator itMean = mean.begin(); itMean != mean.end(); ++itMean)
       *itMean /= pointCounter;
     
    return pointCounter;

}

/*! @brief Distance between two vectors
 * 
 */
double ossimSDFilter::dist(std::vector< double >& A, std::vector< double >& B)
{
    double result = 0.0;
    for(std::vector<double>::iterator it = A.begin(), it2 = B.begin(); it != A.end(); ++it, ++it2)
      result += (*it - *it2)*(*it - *it2);
    
    return result;
}

/*! @brief Perfroms the mean shift operation and then returns the labelled clusters and the location of the means
 * 
 */
void ossimSDFilter::meanshift(std::vector< double >& data, int rows, int cols, double bw, double rate, int iterMax, std::vector< double >& labelledClusters, std::vector< double >& meansFinal)
{
  float bw2 = bw * bw;      
    int i = 0, j = 0;          
    int delta = 1;
    int nLabelledClusters = 1;
    double epsilon = 0.0001;
    
    std::vector<double> meansCurr;
    std::vector<double> meansNext;
    std::vector<double> meansRow;
    std::vector<int> groupedLabels;
    std::vector<int> deltas;
    std::vector<double> originalData;
    
    //Initialise vectors
    meansCurr.insert(meansCurr.end(),&data[0], &data[rows*cols]);
    meansNext.resize(rows*cols);
    meansRow.resize(rows);
    groupedLabels.resize(cols);
    deltas.resize(cols);
    originalData.resize(rows*cols);
    originalData = meansCurr;
    
    //Fill deltas with 1s and labels with 0
    std::fill(deltas.begin(),deltas.end(),1);  
    std::fill(groupedLabels.begin(), groupedLabels.end(), 0);
    std::fill(labelledClusters.begin(), labelledClusters.begin(), 0);
    
    for(int itercount = 0;itercount < iterMax && delta; itercount++)
    {
      
      //Calculate next mean by shifting current data
      i = 0;
      for(std::vector<int>::iterator itCurrentDelta = deltas.begin(); itCurrentDelta != deltas.end(); ++itCurrentDelta, ++i)      
      {
	if(*itCurrentDelta > 0)
	{
	  std::vector<double> subVectorCurr(meansCurr.begin()+i*rows,meansCurr.end());
	  if(meanvector(subVectorCurr, originalData, rows, cols, bw2, meansRow) > 0) 
	  {
	   j = 0; 
           for(std::vector<double>::iterator itCurrentRowMean = meansRow.begin(); itCurrentRowMean != meansRow.end(); ++itCurrentRowMean, ++j)
	     meansNext.at(i*rows+j) = (1-rate)*meansCurr.at(i*rows+j) + rate*(*itCurrentRowMean);
          } 
          else
	  {
	   j = 0; 
           for(std::vector<double>::iterator itCurrentRowMean = meansRow.begin(); itCurrentRowMean != meansRow.end(); ++itCurrentRowMean, ++j)
	     meansNext.at(i*rows+j) = meansCurr.at(i*rows+j);
          }
 	}
      }

      delta = 0;
      i = 0;
      for(std::vector<int>::iterator itCurrentDelta = deltas.begin(); itCurrentDelta != deltas.end(); ++itCurrentDelta, ++i)
      {
         std::vector<double> subVectorCurr(meansCurr.begin()+i*rows,meansCurr.end());
         std::vector<double> subVectorNext(meansNext.begin()+i*rows,meansNext.end());
	 if(*itCurrentDelta > 0 && dist(subVectorNext,subVectorCurr) > epsilon)
          delta = 1;
         else
          *itCurrentDelta = 0;
      }
	meansCurr = meansNext;
    }
    
    
    i = 0;
    for(std::vector<int>::iterator itCurrentLabel = groupedLabels.begin(); itCurrentLabel != groupedLabels.end(); ++itCurrentLabel, ++i)
    {
       if(*itCurrentLabel == 0)
       {
	 j = 0;
	 for(std::vector<int>::iterator itCurrentLabelInner = groupedLabels.begin(); itCurrentLabelInner != groupedLabels.end(); ++itCurrentLabelInner, ++j)
	 {
	  std::vector<double> subVectorCurr(meansCurr.begin()+j*rows,meansCurr.end());
	  std::vector<double> subVectorNext(meansNext.begin()+i*rows,meansNext.end());
   	  
	  if(*itCurrentLabelInner == 0 && dist(subVectorNext,subVectorCurr) < bw2)
          {
            labelledClusters.at(j) = nLabelledClusters;
            groupedLabels.at(j) = 1;
          }  
	 }
         nLabelledClusters++;
    	}
    }
    
    nLabelledClusters = nLabelledClusters-1;
    meansFinal = meansNext; 
}
