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

#include "ossimSimpleFilter.h"

RTTI_DEF1(ossimSimpleFilter, "ossimSimpleFilter", ossimImageSourceFilter)

ossimSimpleFilter::ossimSimpleFilter(ossimObject* owner)
   :ossimImageSourceFilter(owner)
{
}

ossimSimpleFilter::ossimSimpleFilter(ossimImageSource* inputSource)
   : ossimImageSourceFilter(NULL, inputSource),
     outputTile(NULL)
{
}

ossimSimpleFilter::~ossimSimpleFilter()
{
}

ossimRefPtr<ossimImageData> ossimSimpleFilter::getTile(const ossimIrect& tileRect,
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

void ossimSimpleFilter::initialize()
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

ossimScalarType ossimSimpleFilter::getOutputScalarType() const
{
   if(!isSourceEnabled())
   {
      return ossimImageSourceFilter::getOutputScalarType();
   }
   
   return OSSIM_UCHAR;
}

ossim_uint32 ossimSimpleFilter::getNumberOfOutputBands() const
{
   if(!isSourceEnabled())
   {
      return ossimImageSourceFilter::getNumberOfOutputBands();
   }
   return theInputConnection->getNumberOfOutputBands();
}

bool ossimSimpleFilter::saveState(ossimKeywordlist& kwl,  const char* prefix)const
{
   ossimImageSourceFilter::saveState(kwl, prefix);

   kwl.add(prefix,"aperture_size",0,true);
   
   return true;
}

bool ossimSimpleFilter::loadState(const ossimKeywordlist& kwl, const char* prefix)
{
   ossimImageSourceFilter::loadState(kwl, prefix);

   const char* lookup = kwl.find(prefix, "aperture_size");
   if(lookup)
   {
//       setApertureSize(ossimString(lookup).toInt());
   }
   return true;
}

void ossimSimpleFilter::runUcharTransformation(ossimImageData* tile) {
   
	int nChannels = tile->getNumberOfBands();
	
	for(int k=0; k<nChannels; k++) {
	  
		// Get the correct buffer (input) pointer
		ossim_uint16 *inBuf = (ossim_uint16*)tile->getBuf(k);

		// Grab output buffer
		uchar *outBuf = (uchar*)outputTile->getBuf(k);
		
		for (unsigned int i = 0; i < tile->getWidth(); i++)
		{
		  for (unsigned int j = 0; j < tile->getHeight(); j++)
		  {
		      	*outBuf = (uchar) (*inBuf/scaleValue);
			++inBuf;
			++outBuf;		    
		  }
		}
	}

	outputTile->validate(); 
}


void ossimSimpleFilter::setProperty(ossimRefPtr<ossimProperty> property)

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



ossimRefPtr<ossimProperty> ossimSimpleFilter::getProperty(const ossimString& name)const

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



void ossimSimpleFilter::getPropertyNames(std::vector<ossimString>& propertyNames)const

{

        ossimImageSourceFilter::getPropertyNames(propertyNames);

        propertyNames.push_back("aperture_size");

}