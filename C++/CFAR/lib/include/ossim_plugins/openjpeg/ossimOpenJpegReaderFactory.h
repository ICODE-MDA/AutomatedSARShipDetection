//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  David Burken
//
// Description: Factory for OSSIM Open JPEG reader.
//----------------------------------------------------------------------------
// $Id: ossimOpenJpegReaderFactory.h 10110 2006-12-14 18:20:54Z dburken $
#ifndef ossimOpenJpegReaderFactory_HEADER
#define ossimOpenJpegReaderFactory_HEADER

#include <ossim/imaging/ossimImageHandlerFactoryBase.h>

class ossimString;
class ossimFilename;
class ossimKeywordlist;

/** @brief Factory for PNG image reader. */
class ossimOpenJpegReaderFactory : public ossimImageHandlerFactoryBase
{
public:

   /** @brief virtual destructor */
   virtual ~ossimOpenJpegReaderFactory();

   /**
    * @brief static method to return instance (the only one) of this class.
    * @return pointer to instance of this class.
    */
   static ossimOpenJpegReaderFactory* instance();

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
    * @brief createObject that takes a class name (ossimOpenJpegReader)
    * @param typeName Should be "ossimOpenJpegReader".
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
    * @brief Adds ossimOpenJpegWriter to the typeList.
    * @param typeList List to add to.
    */
   virtual void getTypeNameList(std::vector<ossimString>& typeList)const;

   /**
    * @brief Method to add supported extension to the list, like "png".
    *
    * @param extensionList The list to add to.
    */
   virtual void getSupportedExtensions(
      ossimImageHandlerFactoryBase::UniqueStringList& extensionList)const;
  
protected:
   /** @brief hidden from use default constructor */
   ossimOpenJpegReaderFactory();

   /** @brief hidden from use copy constructor */
   ossimOpenJpegReaderFactory(const ossimOpenJpegReaderFactory&);

   /** @brief hidden from use copy constructor */
   void operator=(const ossimOpenJpegReaderFactory&);

   /** static instance of this class */
   static ossimOpenJpegReaderFactory* theInstance;

TYPE_DATA
};

#endif /* end of #ifndef ossimOpenJpegReaderFactory_HEADER */
