//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  David Burken
//
// Description: Factory for OSSIM Portable Document Format (PDF) writer.
//----------------------------------------------------------------------------
// $Id$

#include "ossimPdfWriterFactory.h"
#include "ossimPdfWriter.h"
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageFileWriter.h>

ossimPdfWriterFactory* ossimPdfWriterFactory::theInstance = 0;

RTTI_DEF1(ossimPdfWriterFactory,
          "ossimPdfWriterFactory",
          ossimImageWriterFactoryBase);

ossimPdfWriterFactory::~ossimPdfWriterFactory()
{
   theInstance = 0;
}

ossimPdfWriterFactory* ossimPdfWriterFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new ossimPdfWriterFactory;
   }
   return theInstance;
}

ossimImageFileWriter *ossimPdfWriterFactory::createWriterFromExtension(
   const ossimString& fileExtension)const
{
   ossimRefPtr<ossimPdfWriter> writer = 0;
   if ( (fileExtension == "pdf") || (fileExtension == ".pdf") )
   {
      writer = new ossimPdfWriter;
   }
   return writer.release();
}

ossimImageFileWriter*
ossimPdfWriterFactory::createWriter(const ossimKeywordlist& kwl,
                                    const char *prefix)const
{
   ossimRefPtr<ossimImageFileWriter> writer = 0;
   const char* type = kwl.find(prefix, ossimKeywordNames::TYPE_KW);
   if (type)
   {
      writer = createWriter(ossimString(type));
      if ( writer.valid() )
      {
         if (writer->loadState(kwl, prefix) == false)
         {
            writer = 0;
         }
      }
   }
   return writer.release();
}

ossimImageFileWriter* ossimPdfWriterFactory::createWriter(
   const ossimString& typeName)const
{
   ossimRefPtr<ossimImageFileWriter> writer = 0;
   if (typeName == "ossimPdfWriter")
   {
      writer = new ossimPdfWriter;
   }
   else
   {
      // See if the type name is supported by the writer.
      writer = new ossimPdfWriter;
      if ( writer->hasImageType(typeName) == false )
      {
         writer = 0;
      }
   }
   return writer.release();
}

ossimObject* ossimPdfWriterFactory::createObject(const ossimKeywordlist& kwl,
                                                 const char *prefix)const
{
   return createWriter(kwl, prefix);
}

ossimObject* ossimPdfWriterFactory::createObject(
   const ossimString& typeName) const
{
   return createWriter(typeName);
}

void ossimPdfWriterFactory::getExtensions(
   std::vector<ossimString>& result)const
{
   result.push_back("pdf");
}

void ossimPdfWriterFactory::getTypeNameList(std::vector<ossimString>& typeList)const
{
   typeList.push_back(ossimString("ossimPdfWriter"));
}

void ossimPdfWriterFactory::getImageTypeList(std::vector<ossimString>& imageTypeList)const
{
   ossimRefPtr<ossimPdfWriter> writer = new ossimPdfWriter;
   writer->getImageTypeList(imageTypeList);
   writer = 0;
}

void ossimPdfWriterFactory::getImageFileWritersBySuffix(ossimImageWriterFactoryBase::ImageFileWriterList& result,
                                                        const ossimString& ext)const
{
   ossimString testExt = ext.downcase();
   if(testExt == "pdf")
   {
      result.push_back(new ossimPdfWriter);
   }
}

void ossimPdfWriterFactory::getImageFileWritersByMimeType(ossimImageWriterFactoryBase::ImageFileWriterList& result,
                                                          const ossimString& mimeType)const
{
   ossimString testMime = mimeType.downcase();
   if(testMime == "application/pdf")
   {
      result.push_back(new ossimPdfWriter);
   }
}

ossimPdfWriterFactory::ossimPdfWriterFactory(){}

ossimPdfWriterFactory::ossimPdfWriterFactory(const ossimPdfWriterFactory&){}

void ossimPdfWriterFactory::operator=(const ossimPdfWriterFactory&){}




