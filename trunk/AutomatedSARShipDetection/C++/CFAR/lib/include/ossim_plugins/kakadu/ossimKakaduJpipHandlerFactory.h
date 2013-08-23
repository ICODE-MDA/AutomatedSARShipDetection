//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  Garrett Potts
//
// Description: Factory for OSSIM JPIP reader using kakadu library.
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimKakaduJpipHandlerFactory_HEADER
#define ossimKakaduJpipHandlerFactory_HEADER 1

#include <ossim/imaging/ossimImageHandlerFactoryBase.h>

class ossimString;
class ossimFilename;
class ossimKeywordlist;

/** @brief Factory for J2K image reader. */
class ossimKakaduJpipHandlerFactory : public ossimImageHandlerFactoryBase
{
public:

   /** @brief virtual destructor */
   virtual ~ossimKakaduJpipHandlerFactory();

   /**
    * @brief static method to return instance (the only one) of this class.
    * @return pointer to instance of this class.
    */
   static ossimKakaduJpipHandlerFactory* instance();

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
    * @brief createObject that takes a class name (ossimKakaduReader)
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
    * @brief Adds ossimKakaduWriter to the typeList.
    * @param typeList List to add to.
    */
   virtual void getTypeNameList(std::vector<ossimString>& typeList)const;

   /**
    * @brief Method to add supported extension to the list, like "jp2".
    *
    * @param extensionList The list to add to.
    */
   virtual void getSupportedExtensions(
      ossimImageHandlerFactoryBase::UniqueStringList& extensionList)const;
  
   /**
    *
    * Will add to the result list any handler that supports the passed in extensions
    *
    */
   virtual void getImageHandlersBySuffix(ossimImageHandlerFactoryBase::ImageHandlerList& result,
                                         const ossimString& ext)const;
   /**
    *
    * Will add to the result list and handler that supports the passed in mime type
    *
    */
   virtual void getImageHandlersByMimeType(ossimImageHandlerFactoryBase::ImageHandlerList& result,
                                           const ossimString& mimeType)const;
protected:

   /**
    * @brief Method to weed out extensions that this plugin knows it does
    * not support.  This is to avoid a costly open on say a tiff or jpeg that
    * is not handled by this plugin.
    *
    * @return true if extension, false if not.
    */
  // bool hasExcludedExtension(const ossimFilename& file) const;
   
   /** @brief hidden from use default constructor */
   ossimKakaduJpipHandlerFactory();

   /** @brief hidden from use copy constructor */
   ossimKakaduJpipHandlerFactory(const ossimKakaduJpipHandlerFactory&);

   /** @brief hidden from use copy constructor */
   void operator=(const ossimKakaduJpipHandlerFactory&);

   /** static instance of this class */
   static ossimKakaduJpipHandlerFactory* m_instance;

TYPE_DATA
};

#endif /* end of #ifndef ossimKakaduJpipHandlerFactory_HEADER */
