//----------------------------------------------------------------------------
// Copyright (c) 2005, Garrett Potts, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description: Constants file for ossim plugins.
//
// $Id: ossimRegistrationExports.h 8366 2005-11-17 13:34:03Z fclaudel $
//----------------------------------------------------------------------------
#ifndef ossimRegistrationExports_HEADER
#define ossimRegistrationExports_HEADER

#if defined(__MINGW32__) || defined(__CYGWIN__) || defined(_MSC_VER) || defined(__VISUALC__) || defined(__BORLANDC__) || defined(__WATCOMC__)
#  define OSSIM_REGISTRATION_EXPORT __declspec(dllexport)
#  define OSSIM_REGISTRATION_IMPORT __declspec(dllexport)

#  ifdef OSSIM_REGISTRATION_MAKINGDLL
#     define OSSIM_REGISTRATION_DLL OSSIM_REGISTRATION_EXPORT
#  else
#     define  OSSIM_REGISTRATION_DLL OSSIM_REGISTRATION_IMPORT
#  endif
#else
# define OSSIM_REGISTRATION_DLL
#endif

#endif /* #ifndef ossimRegistrationExports_HEADER */
