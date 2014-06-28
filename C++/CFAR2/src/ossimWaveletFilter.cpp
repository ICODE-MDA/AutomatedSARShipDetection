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

#include "ossimWaveletFilter.h"

RTTI_DEF1(ossimWaveletFilter, "ossimWaveletFilter", ossimImageSourceFilter)

ossimWaveletFilter::ossimWaveletFilter(ossimObject* owner)
   :ossimImageSourceFilter(owner)
{
}

ossimWaveletFilter::ossimWaveletFilter(ossimImageSource* inputSource)
   : ossimImageSourceFilter(NULL, inputSource),
     outputTile(NULL)
{
}

ossimWaveletFilter::~ossimWaveletFilter()
{
}

ossimRefPtr<ossimImageData> ossimWaveletFilter::getTile(const ossimIrect& tileRect,
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

void ossimWaveletFilter::initialize()
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

ossimScalarType ossimWaveletFilter::getOutputScalarType() const
{
   if(!isSourceEnabled())
   {
      return ossimImageSourceFilter::getOutputScalarType();
   }
   
   return OSSIM_UCHAR;
}

ossim_uint32 ossimWaveletFilter::getNumberOfOutputBands() const
{
   if(!isSourceEnabled())
   {
      return ossimImageSourceFilter::getNumberOfOutputBands();
   }
   return theInputConnection->getNumberOfOutputBands();
}

bool ossimWaveletFilter::saveState(ossimKeywordlist& kwl,  const char* prefix)const
{
   ossimImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix,"aperture_size",0,true);
   
   return true;
}

bool ossimWaveletFilter::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   ossimImageSourceFilter::loadState(kwl, prefix);

   const char* lookup = kwl.find(prefix, "aperture_size");
   if(lookup)
   {
//       setApertureSize(ossimString(lookup).toInt());
   }
   return true;
}

void ossimWaveletFilter::runUcharTransformation(ossimImageData* tile) {
		
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
		
		// Threshold image using Wavelet
		cv::Mat inputClone;
		inputClone = inputTile.clone();
		simpleWavelet(inputClone, inputTile);

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

void ossimWaveletFilter::simpleWavelet(cv::Mat& inputImage, cv::Mat& outputImage)
{
  //Check that input/output images are 8-bit grayscale single channel images
  //NOTE: outputImage will be a binary image where TRUE == 255 and FALSE = 0
  assert(inputImage.type() == CV_8UC1);

  //Get image height/width
  int width = inputImage.cols;
  int height = inputImage.rows;

  //Create empty matrices for processing
  //NOTE: coeffienct matrices are half the height/width of input image
  cv::Mat sourceImage = cv::Mat(height, width, CV_32FC1),
          cA = cv::Mat(height/2, width/2, CV_32FC1),
          cV = cv::Mat(height/2, width/2, CV_32FC1),
	  cH = cv::Mat(height/2, width/2, CV_32FC1),
          cD = cv::Mat(height/2, width/2, CV_32FC1);

  //Convert input image to floating and then get four coeffient images
  inputImage.clone().convertTo(sourceImage,CV_32FC1);
  assert(sourceImage.type() == CV_32FC1);
  getHaarWaveletCoeff(sourceImage,cA,cH,cV,cD);

  //Prepare the output image (scale, resize and multiply)
  cv::Mat rImage = cA.clone();
  outputImage = cv::Mat(rImage.rows, rImage.cols, CV_8UC1);

  //Scale outputs
  scaleImage(cA);
  scaleImage(cV);
  scaleImage(cH);
  scaleImage(cD);  

  //Prepare final images (R = W * W_V * W_H * W_D; from dissertation).
  rImage = cA.mul(cV);
  rImage = rImage.mul(cH);
  rImage = rImage.mul(cD);

  //Find mean and standard deviation then threshold rImage to get final binary output image
  cv::Scalar mean, stddev;
  cv::meanStdDev(rImage, mean, stddev);
  double threshold = mean[0] + cThreshold*stddev[0]; // mu + c*sigma
  cv::threshold(rImage,outputImage,threshold,255,0);

  //Ensure that output image is the same size as input image (resize each dimension by embiggening it by a factor 2 or so)
  cv::resize(outputImage, outputImage, inputImage.size(), 0, 0, CV_INTER_AREA);
}

void ossimWaveletFilter::getHaarWaveletCoeff(cv::Mat &src, cv::Mat &cA, cv::Mat &cH, cv::Mat &cV, cv::Mat &cD)
{
    // Check that the input and the four output images are all 32 bit floating matrices
    assert(src.type() == CV_32FC1 && cA.type() == CV_32FC1 && cH.type() == CV_32FC1 && cV.type() == CV_32FC1 && cD.type() == CV_32FC1);

    // For each pixel calculate the haar wavelet coefficients. cA is the average pixel image, cV,cH and cD are the vertical, horizontal and diagonal pixel coefficent images.
    // NOTE: The results of this process generate coefficent images that are half the size in the x & y directions. To allow for comparison to the other methods it is recommended either the image is resized to twice the size before this function or after it. 
    for (int y=0;y<(src.rows>>1);y++)
        {
            for (int x=0; x<(src.cols>>1);x++)
            {
                cA.at<float>(y,x)=(src.at<float>(2*y,2*x)
				  +src.at<float>(2*y,2*x+1)
				  +src.at<float>(2*y+1,2*x)
				  +src.at<float>(2*y+1,2*x+1))*0.5;

		cH.at<float>(y,x)=(src.at<float>(2*y,2*x)
				  +src.at<float>(2*y+1,2*x)
				  -src.at<float>(2*y,2*x+1)
				  -src.at<float>(2*y+1,2*x+1))*0.5;

		cV.at<float>(y,x)=(src.at<float>(2*y,2*x)
				  +src.at<float>(2*y,2*x+1)
				  -src.at<float>(2*y+1,2*x)
				  -src.at<float>(2*y+1,2*x+1))*0.5;

		cD.at<float>(y,x)=(src.at<float>(2*y,2*x)
				  -src.at<float>(2*y,2*x+1)
				  -src.at<float>(2*y+1,2*x)
				  +src.at<float>(2*y+1,2*x+1))*0.5;
            }
        }
}

void ossimWaveletFilter::scaleImage(cv::Mat& image)
{
  //Scale data between 0 ~ 1 for multiplication (ensures each image is weighted evenly)
  double m = 0, M = 0;
  cv::minMaxLoc(image,&m,&M);
  if((M-m)>0) 
   {image=image*(1.0/(M-m))-m/(M-m);}
}

void ossimWaveletFilter::setProperty(ossimRefPtr<ossimProperty> property)

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



ossimRefPtr<ossimProperty> ossimWaveletFilter::getProperty(const ossimString& name)const

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



void ossimWaveletFilter::getPropertyNames(std::vector<ossimString>& propertyNames)const

{

        ossimImageSourceFilter::getPropertyNames(propertyNames);

        propertyNames.push_back("aperture_size");

}
