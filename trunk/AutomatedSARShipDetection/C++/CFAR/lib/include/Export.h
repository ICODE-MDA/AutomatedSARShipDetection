/* -*-c++-*- libwms - Copyright (C) since 2004 Garrett Potts 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the libwms Public License (WMSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * libwms Public License for more details.
*/
#ifndef ossimGuiExport_HEADER
#define ossimGuiExport_HEADER
// define used to include in API which is being fazed out
// if you can compile your apps with this turned off you are
// well placed for compatablity with future versions.
//#define USE_DEPRECATED_API

#if defined(_MSC_VER)
    #pragma warning( disable : 4244 )
    #pragma warning( disable : 4251 )
    #pragma warning( disable : 4267 )
    #pragma warning( disable : 4275 )
    #pragma warning( disable : 4290 )
    #pragma warning( disable : 4786 )
    #pragma warning( disable : 4305 )
#endif

#if defined(_MSC_VER) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#    ifdef OSSIMGUI_LIBRARY
#        define OSSIMGUI_EXPORT   __declspec(dllexport)
#        define OSSIMGUI_DLL   OSSIMGUI_EXPORT
#    else
#        define OSSIMGUI_EXPORT   __declspec(dllimport)
#        define OSSIMGUI_DLL   OSSIMGUI_EXPORT
#    endif /* OSSIMPLANETQT_LIBRARY */
#else
#    define OSSIMGUI_EXPORT
#    define OSSIMGUI_DLL   OSSIMGUI_EXPORT
#endif  

// set up define for whether member templates are supported by VisualStudio compilers.
#ifdef _MSC_VER
# if (_MSC_VER >= 1300)
#  define __STL_MEMBER_TEMPLATES
# endif
#endif



#endif
