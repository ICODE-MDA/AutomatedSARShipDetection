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

#include "ossimCFARFilter.h"

RTTI_DEF1(ossimCFARFilter, "ossimCFARFilter", ossimImageSourceFilter)

ossimCFARFilter::ossimCFARFilter(ossimObject* owner)
   :ossimImageSourceFilter(owner)
{
}

ossimCFARFilter::ossimCFARFilter(ossimImageSource* inputSource)
   : ossimImageSourceFilter(NULL, inputSource),
     outputTile(NULL)
{
}

ossimCFARFilter::~ossimCFARFilter()
{
}

ossimRefPtr<ossimImageData> ossimCFARFilter::getTile(const ossimIrect& tileRect,
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

void ossimCFARFilter::initialize()
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

ossimScalarType ossimCFARFilter::getOutputScalarType() const
{
   if(!isSourceEnabled())
   {
      return ossimImageSourceFilter::getOutputScalarType();
   }
   
   return OSSIM_UCHAR;
}

ossim_uint32 ossimCFARFilter::getNumberOfOutputBands() const
{
   if(!isSourceEnabled())
   {
      return ossimImageSourceFilter::getNumberOfOutputBands();
   }
   return theInputConnection->getNumberOfOutputBands();
}

bool ossimCFARFilter::saveState(ossimKeywordlist& kwl,  const char* prefix)const
{
   ossimImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix,"aperture_size",0,true);
   
   return true;
}

bool ossimCFARFilter::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   ossimImageSourceFilter::loadState(kwl, prefix);

   const char* lookup = kwl.find(prefix, "aperture_size");
   if(lookup)
   {
//       setApertureSize(ossimString(lookup).toInt());
   }
   return true;
}

void ossimCFARFilter::runUcharTransformation(ossimImageData* tile) {
		
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
		
		// Threshold image using CFAR
		cv::Mat inputClone;
		inputClone = inputTile.clone();
		simpleCFAR(inputClone, inputTile);

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

void ossimCFARFilter::simpleCFAR(cv::Mat& inputImage, cv::Mat& outputImage)
{
  cv::Mat outputImageFinal;
   
  outputImage.create(inputImage.rows,inputImage.cols, inputImage.type());
  outputImage.setTo(cv::Scalar::zeros());
  outputImageFinal.create(inputImage.rows,inputImage.cols, inputImage.type());
  outputImageFinal.setTo(cv::Scalar::zeros());
  
  int borderSize = 100, pixel = 255;
    
  int top = borderSize, left = borderSize, bottom = borderSize, right = borderSize;
  std::vector<int> neighbours;
  
  /// Add border (zeros) for actual image border processing
  cv::copyMakeBorder(inputImage, outputImage, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar::all(0));
  outputImage.copyTo(outputImageFinal);
  
  if(cfarMethod == 0)
  {
    /// Run through buffer while simulaneously filling the OpenCV matrix/image (raster).
    for (int i = borderSize; i < outputImage.rows - borderSize; i++)  
    {
	    for (int j = borderSize; j < outputImage.cols - borderSize; j++)
	    {	
		    // Centre pixel
		    pixel = outputImage.at<uint8_t>(i,j);
		    
		    if(pixel != 0)
		    { 
		      // Get guard and neighbour rectangles  cout 
		      // Make a copy of sub-image ('normal').
		      // Binary image must be the same size to multiply.
		      cv::Rect rect = cv::Rect(j - neighbourSize/2, i - neighbourSize/2, neighbourSize, neighbourSize);
		      cv::Mat nImage = outputImage(rect).clone();
		      cv::Mat guardBinaryImage = outputImage(rect).clone();
		      
		      // Switch on all non-guard pixels (1) and the guard area must be off (0).
		      guardBinaryImage.setTo(cv::Scalar::all(1));
		      rect = cv::Rect(guardBinaryImage.rows/2 - guardSize/2, guardBinaryImage.cols/2 - guardSize/2, guardSize, guardSize);
		      cv::Mat guardPixels = guardBinaryImage(rect);
		      guardPixels.setTo(cv::Scalar::all(0));

		      // Multiply the original image by the binary gaurd image to get the 'mask' of processable pixel value (valid neighbours). 
		      cv::Mat mask;
		      mask = guardBinaryImage.clone();
		      cv::multiply(nImage,guardBinaryImage,mask);
		      
		      // Use OpenCV + mask to find only valid neighbours mean pixel value	  
		      cv::Scalar mean = cv::mean(nImage,mask);
    
		      // Mark on new image using CFAR.
		      if(pixel > thresholdValue*mean[0])
			outputImageFinal.at<uint8_t>(i,j) = 255;
		      else
			outputImageFinal.at<uint8_t>(i,j) = 0;
		    }
		    else
		    {
		      outputImageFinal.at<uint8_t>(i,j) = 0;
		    }
    
	    }
    }
  }
  else
  {
    /// Run through buffer while simulaneously filling the OpenCV matrix/image (raster).
    for (int i = borderSize; i < outputImage.rows - borderSize; i++)  
    {
	    for (int j = borderSize; j < outputImage.cols - borderSize; j++)
	    {	
		    double sum = 0.0, avg = 0.0;
		    
		    // Centre pixel
		    pixel = outputImage.at<uint8_t>(i,j);
		    
		    if(pixel != 0)
		    { 
		      for(int x = -floor(neighbourSize/2); x <= floor(neighbourSize/2); x++)
		      {
			  for(int y = -floor(neighbourSize/2); y <= floor(neighbourSize/2); y++)
			  {
			      sum += (int) outputImage.at<uint8_t>(i+y, j+x);
			  }
		      }

		      sum -= pixel;

		      for(int x = -floor(guardSize/2); x <= floor(guardSize/2); x++)
		      {
			  for(int y = -floor(guardSize/2); y <= floor(guardSize/2); y++)
			  {
			      sum -= (int) outputImage.at<uint8_t>(i+y, j+x);
			  }
		      }

		      sum += pixel;
		      
		      avg = sum/(neighbourSize*neighbourSize - guardSize*guardSize);
    
		      // Mark on new image using CFAR.
		      if(pixel > thresholdValue*avg)
			outputImageFinal.at<uint8_t>(i,j) = 255;
		      else
			outputImageFinal.at<uint8_t>(i,j) = 0;
		    }
		    else
		    {
		      outputImageFinal.at<uint8_t>(i,j) = 0;
		    }
    
	    }
    }
  }
  /// Remove border
  cv::Rect borderlessRect = cv::Rect(borderSize, borderSize, outputImage.cols - borderSize - borderSize, outputImage.rows - borderSize - borderSize);
  outputImage = outputImageFinal(borderlessRect).clone();
}


void ossimCFARFilter::setProperty(ossimRefPtr<ossimProperty> property)

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



ossimRefPtr<ossimProperty> ossimCFARFilter::getProperty(const ossimString& name)const

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



void ossimCFARFilter::getPropertyNames(std::vector<ossimString>& propertyNames)const

{

        ossimImageSourceFilter::getPropertyNames(propertyNames);

        propertyNames.push_back("aperture_size");

}
