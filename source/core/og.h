/*******************************************************************************
#      ____               __          __  _      _____ _       _               #
#     / __ \              \ \        / / | |    / ____| |     | |              #
#    | |  | |_ __   ___ _ __ \  /\  / /__| |__ | |  __| | ___ | |__   ___      #
#    | |  | | '_ \ / _ \ '_ \ \/  \/ / _ \ '_ \| | |_ | |/ _ \| '_ \ / _ \     #
#    | |__| | |_) |  __/ | | \  /\  /  __/ |_) | |__| | | (_) | |_) |  __/     #
#     \____/| .__/ \___|_| |_|\/  \/ \___|_.__/ \_____|_|\___/|_.__/ \___|     #
#           | |                                                                #
#           |_|                                                                #
#                                                                              #
#                                (c) 2011 by                                   #
#           University of Applied Sciences Northwestern Switzerland            #
#                     Institute of Geomatics Engineering                       #
#                           martin.christen@fhnw.ch                            #
********************************************************************************
*     Licensed under MIT License. Read the file LICENSE for more information   *
*******************************************************************************/

#ifndef _OG_H
#define _OG_H

#ifdef _MSC_VER
#pragma warning(disable:4251)
#pragma warning(disable:4275)
#ifndef OPENGLOBE_API
 #ifdef BUILD_OPENGLOBELIB
 #define OPENGLOBE_API __declspec(dllexport)
 #else
 #define OPENGLOBE_API __declspec(dllimport)
 #endif
#endif
#else
#define OPENGLOBE_API
#endif

#ifndef OS_NAME
#if defined(macintosh)
#  define OS_MACOS
#  define OS_NAME L"MacOS Classic"
#elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MACOSX
#  define OS_NAME L"MacOS X"
#elif defined(WINDOWS) || defined(_WIN32)
#  define OS_WINDOWS
#ifdef _WIN64
#  define OS_WINDOWS64
#  define OS_NAME L"Windows 64bit"
#else 
#  define OS_WINDOWS32
#  define OS_NAME L"Windows"
#endif
#elif defined(linux)
#  define OS_LINUX
#  define OS_NAME L"Linux"
#elif defined(__FreeBSD__)
#  define OS_FREEBSD
#  define OS_NAME L"FreeBSD"
#elif defined(__sun__)
#  define OS_SUN
#  define OS_NAME L"SunOS"
#else
#  define OS_UNKNOWN
#  define OS_NAME L"Unsupported OS"
#endif
#endif


#ifndef LLCONST
#ifdef _MSC_EXTENSIONS
typedef __int64	            int64;
typedef unsigned __int64      uint64;
#define LLCONST(a) (a##i64)
#define ULLCONST(a) (a##ui64)
//#elif !defined(_MSC_VER)
//typedef int64_t  	            int64;
//typedef uint64_t              uint64;
//#define LLCONST(a) (a##i64)
//#define ULLCONST(a) (a##ui64)
#else
typedef long long	            int64;
typedef unsigned long long    uint64;
typedef long long __int64;  // do not use
#define LLCONST(a) (a##ll)
#define ULLCONST(a) (a##ull)
#endif
#endif

//------------------------------------------------------------------------------
namespace math
{
   template<typename T>
   inline T Min(T a, T b)    //!< Minimum (a,b)
   {
      return (a < b ? a : b);
   }
   //---------------------------------------------------------------------------
   template<typename T>
   inline T Max(T a, T b)    //!< Maximum (a,b)
   {
      return (a < b ? b : a);
   }

};

#endif
