//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  Mingjie Su
//
// Description: Factory for OSSIM GeoPdf reader using Lizard library.
//----------------------------------------------------------------------------
// $Id: ossimGeoPdfReaderFactory.h 20935 2012-05-18 14:19:30Z dburken $

#ifndef ossimGeoPdfReaderFactory_HEADER
#define ossimGeoPdfReaderFactory_HEADER

#include <ossim/imaging/ossimImageHandlerFactoryBase.h>

class ossimString;
class ossimFilename;
class ossimKeywordlist;

/** @brief Factory for GeoPdf image reader. */
class ossimGeoPdfReaderFactory : public ossimImageHandlerFactoryBase
{
public:

   /** @brief virtual destructor */
   virtual ~ossimGeoPdfReaderFactory();

   /**
    * @brief static method to return instance (the only one) of this class.
    * @return pointer to instance of this class.
    */
   static ossimGeoPdfReaderFactory* instance();

   /**
    * @brief open that takes a file name.
    * @param file The file to open.
    * @param openOverview If true image handler will attempt to open overview.
    * default = true 
    * @return pointer to image handler on success, NULL on failure.
    */
   virtual ossimImageHandler* open(const ossimFilename& fileName,
                                   bool openOverview=true) const;

   /**
    * @brief open that takes a keyword list and prefix.
    * @param kwl The keyword list.
    * @param prefix the keyword list prefix.
    * @return pointer to image handler on success, NULL on failure.
    */
   virtual ossimImageHandler* open(const ossimKeywordlist& kwl,
                                   const char* prefix=0)const;

   /**
    * @brief createObject that takes a class name (ossimGeoPdfReader)
    * @param typeName Should be "ossimKakaduReader".
    * @return pointer to image writer on success, NULL on failure.
    */
   virtual ossimObject* createObject(const ossimString& typeName)const;
   
   /**
    * @brief Creates and object given a keyword list and prefix.
    * @param kwl The keyword list.
    * @param prefix the keyword list prefix.
    * @return pointer to image handler on success, NULL on failure.
    */
   virtual ossimObject* createObject(const ossimKeywordlist& kwl,
                                     const char* prefix=0)const;
   
   /**
    * @brief Adds ossimGeoPdfWriter to the typeList.
    * @param typeList List to add to.
    */
   virtual void getTypeNameList(std::vector<ossimString>& typeList)const;

   /**
    * @brief Method to add supported extension to the list, like "pdf".
    *
    * @param extensionList The list to add to.
    */
   virtual void getSupportedExtensions(
      ossimImageHandlerFactoryBase::UniqueStringList& extensionList)const;

   virtual void getImageHandlersBySuffix(ImageHandlerList& result,
      const ossimString& ext)const;
   virtual void getImageHandlersByMimeType(ImageHandlerList& result,
      const ossimString& mimeType)const;
  
protected:

   /**
    * @brief Method to weed out extensions that this plugin knows it does
    * not support.  This is to avoid a costly open on say a tiff or jpeg that
    * is not handled by this plugin.
    *
    * @return true if extension, false if not.
    */
   bool hasExcludedExtension(const ossimFilename& file) const;
   
   /** @brief hidden from use default constructor */
   ossimGeoPdfReaderFactory();

   /** @brief hidden from use copy constructor */
   ossimGeoPdfReaderFactory(const ossimGeoPdfReaderFactory&);

   /** @brief hidden from use copy constructor */
   void operator=(const ossimGeoPdfReaderFactory&);

   /** static instance of this class */
   static ossimGeoPdfReaderFactory* theInstance;

TYPE_DATA
};

#endif /* end of #ifndef ossimGeoPdfReaderFactory_HEADER */


