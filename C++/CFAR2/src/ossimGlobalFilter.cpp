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

#include "ossimGlobalFilter.h"

RTTI_DEF1(ossimGlobalFilter, "ossimGlobalFilter", ossimImageSourceFilter)

ossimGlobalFilter::ossimGlobalFilter(ossimObject* owner)
   :ossimImageSourceFilter(owner)
{
}

ossimGlobalFilter::ossimGlobalFilter(ossimImageSource* inputSource)
   : ossimImageSourceFilter(NULL, inputSource),
     outputTile(NULL)
{
}

ossimGlobalFilter::~ossimGlobalFilter()
{
}

ossimRefPtr<ossimImageData> ossimGlobalFilter::getTile(const ossimIrect& tileRect,
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
   
   	return outputTile;
   
}

void ossimGlobalFilter::initialize()
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

ossimScalarType ossimGlobalFilter::getOutputScalarType() const
{
   if(!isSourceEnabled())
   {
      return ossimImageSourceFilter::getOutputScalarType();
   }
   
   return OSSIM_UCHAR;
}

ossim_uint32 ossimGlobalFilter::getNumberOfOutputBands() const
{
   if(!isSourceEnabled())
   {
      return ossimImageSourceFilter::getNumberOfOutputBands();
   }
   return theInputConnection->getNumberOfOutputBands();
}

bool ossimGlobalFilter::saveState(ossimKeywordlist& kwl,  const char* prefix)const
{
   ossimImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix,"aperture_size",0,true);
   
   return true;
}

bool ossimGlobalFilter::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   ossimImageSourceFilter::loadState(kwl, prefix);

   const char* lookup = kwl.find(prefix, "aperture_size");
   if(lookup)
   {
//       setApertureSize(ossimString(lookup).toInt());
   }
   return true;
}

void ossimGlobalFilter::runUcharTransformation(ossimImageData* tile) {
	
// 	/// Get openCV image from the N1 file
// 	/// Use whatever band the user selected
// 	/// Current limitation is that the data type (SHORT, FLOAT, INT) isn't 
// 	/// known in advance for all data types.
// 	/// This program currently uses only the UINT16 (SHORT) data type for
// 	/// ENVISAT ASAR N1 files. This may differ for other satellite data
// 	cv::Mat inputImage, inputTempImage, outputImage;
	
	// Build up OpenCV image
	int nChannels = tile->getNumberOfBands();
	
// 	// Create an OpenCV C++ matrix and set its dimensions to that of the satellite image.
// 	// N1 files have a unsigned 16 bit data type and only use one channel (for the moment)
// 	inputImage.create(tile->getHeight(), tile->getWidth(), CV_16UC1);
// 	inputTempImage.create(tile->getHeight(), tile->getWidth(), CV_32FC1);
// 	outputImage.create(tile->getHeight(), tile->getWidth(), CV_16UC1);
	
	// Run through each channel, get input image, process it then save it to output image
	for(int k=0; k<nChannels; k++) {
	  
		// Grab input tile and create shallow OpenCV image (no deep copying)
		ossim_uint16 *inBuf = (ossim_uint16*)tile->getBuf(k);
		cv::Mat inputTile(tile->getHeight(), tile->getWidth(), CV_16UC1, (ossim_uint16*)inBuf);
		
		// Scale input values by some factor (scaleValue)
		cv::divide(inputTile,cv::Scalar::all(scaleValue),inputTile);

		// Convert input into 8 bit, unsigned single channel
		inputTile.convertTo(inputTile,CV_8UC1);
		
		// Threshold image globally
		cv::threshold(inputTile, inputTile, thresholdValue, 255, cv::THRESH_BINARY);
		
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


void ossimGlobalFilter::setProperty(ossimRefPtr<ossimProperty> property)

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



ossimRefPtr<ossimProperty> ossimGlobalFilter::getProperty(const ossimString& name)const

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



void ossimGlobalFilter::getPropertyNames(std::vector<ossimString>& propertyNames)const

{

        ossimImageSourceFilter::getPropertyNames(propertyNames);

        propertyNames.push_back("aperture_size");

}