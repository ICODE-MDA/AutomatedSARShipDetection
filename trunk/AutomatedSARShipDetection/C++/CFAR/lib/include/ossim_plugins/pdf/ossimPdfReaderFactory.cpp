//----------------------------------------------------------------------------
//
// File: ossimPdfReaderFactory.cpp
// 
// License:  See top level LICENSE.txt file
//
// Author:  David Burken
//
// Description: Reader factory for OSSIM PDF plugin.
//----------------------------------------------------------------------------
// $Id$

#include "ossimPdfReaderFactory.h"
#include "ossimPdfReader.h"
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/base/ossimKeywordNames.h>

static const ossimTrace traceDebug("ossimPdfReaderFactory:debug");

RTTI_DEF1(ossimPdfReaderFactory,
          "ossimPdfReaderFactory",
          ossimImageHandlerFactoryBase);

ossimPdfReaderFactory* ossimPdfReaderFactory::theInstance = 0;

ossimPdfReaderFactory::~ossimPdfReaderFactory()
{
   theInstance = 0;
}

ossimPdfReaderFactory* ossimPdfReaderFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new ossimPdfReaderFactory;
   }
   return theInstance;
}
   
ossimImageHandler* ossimPdfReaderFactory::open(
   const ossimFilename& fileName, bool openOverview)const
{
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimPdfReaderFactory::open(filename) DEBUG: entered..."
         << "\ntrying ossimPdfReader"
         << std::endl;
   }
   
   ossimRefPtr<ossimImageHandler> reader = 0;

   reader = new ossimPdfReader;
   reader->setOpenOverviewFlag(openOverview);
   if(reader->open(fileName) == false)
   {
      reader = 0;
   }
   
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimPdfReaderFactory::open(filename) DEBUG: leaving..."
         << std::endl;
   }
   
   return reader.release();
}

ossimImageHandler* ossimPdfReaderFactory::open(const ossimKeywordlist& kwl,
                                               const char* prefix)const
{
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimPdfReaderFactory::open(kwl, prefix) DEBUG: entered..."
         << "Trying ossimPdfReader"
         << std::endl;
   }

   ossimRefPtr<ossimImageHandler> reader = new ossimPdfReader;
   if(reader->loadState(kwl, prefix) == false)
   {
      reader = 0;
   }
   
   if(traceDebug())
   {
      ossimNotify(ossimNotifyLevel_DEBUG)
         << "ossimPdfReaderFactory::open(kwl, prefix) DEBUG: leaving..."
         << std::endl;
   }
   
   return reader.release();
}

ossimObject* ossimPdfReaderFactory::createObject(
   const ossimString& typeName)const
{
   ossimRefPtr<ossimObject> result = 0;
   if(typeName == "ossimPdfReader")
   {
      result = new ossimPdfReader;
   }
   return result.release();
}

ossimObject* ossimPdfReaderFactory::createObject(const ossimKeywordlist& kwl,
                                                 const char* prefix)const
{
   return this->open(kwl, prefix);
}
 
void ossimPdfReaderFactory::getTypeNameList(std::vector<ossimString>& typeList)const
{
   typeList.push_back(ossimString("ossimPdfReader"));
}

void ossimPdfReaderFactory::getSupportedExtensions(
   ossimImageHandlerFactoryBase::UniqueStringList& extensionList)const
{
   extensionList.push_back(ossimString("pdf"));
}

ossimPdfReaderFactory::ossimPdfReaderFactory(){}

ossimPdfReaderFactory::ossimPdfReaderFactory(const ossimPdfReaderFactory&){}

void ossimPdfReaderFactory::operator=(const ossimPdfReaderFactory&){}
