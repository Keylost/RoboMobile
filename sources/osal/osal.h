/*************************************************************************\
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY *
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE   *
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR *
 * PURPOSE.                                                              *
 *                                                                       *
 * NO PART OF THIS PUBLICATION MAY BE REPRODUCED OR UTILIZED IN ANY FORM *
 * OR BY ANY MEANS, ELECTRONIC OR MECHANICAL, WITHOUT PERMISSION IN      *
 * WRITING FORM FROM THE AUTHOR.                                         *
 *                                                                       *
 * COPYRIGHT (C) 2010-2015 ELECARD STB    ALL RIGHTS RESERVED.           *
\*************************************************************************/
// Author Mikhail.Karev@elecard.ru

#ifndef OSAL__H
#define OSAL__H

/** Compiler macros */

#if defined (__GNUC__)
# define OSAL_COMPILER_GCC
#endif

#if defined (__clang__)
# define OSAL_COMPILER_CLANG
#endif

#if defined (__llvm__)
# define OSAL_COMPILER_LLVM
#endif

#if defined (_MSC_VER)
# define OSAL_COMPILER_MSVC
#endif

/** Operating system macros */

#if defined (__gnu_linux__)
# define OSAL_SYSTEM_LINUX
#endif

#if defined (__ANDROID__)
# define OSAL_SYSTEM_ANDROID
#endif

#if defined (__APPLE__) && defined (__MACH__)
# define OSAL_SYSTEM_DARWIN
# include <TargetConditionals.h>
# if (TARGET_OS_IPHONE == 1)
#  define OSAL_SYSTEM_IPHONEOS
# endif
# if (TARGET_IPHONE_SIMULATOR == 1)
#  undef OSAL_SYSTEM_IPHONEOS
#  define OSAL_SYSTEM_IPHONESIMULATOR
# endif
# if defined (OSAL_SYSTEM_IPHONEOS) || defined (OSAL_SYSTEM_IPHONESIMULATOR)
#  define OSAL_SYSTEM_IOS
# else
#  define OSAL_SYSTEM_OSX
# endif
#endif

#if defined (__pnacl__) || defined (__native_client__)
# define OSAL_SYSTEM_NACL
# if defined (_NEWLIB_VERSION)
#  define OSAL_SYSTEM_NACL_NEWLIB
# else
#  define OSAL_SYSTEM_NACL_GLIBC
# endif
#endif

#if defined (USE_QT) || defined (QT_CORE_LIB) || defined (QT_NETWORK_LIB) || defined(QT_GUI_LIB) || defined (QT_WIDGETS_LIB) || defined (QT_MULTIMEDIA_LIB) || defined (QT_OPENGL_LIB)
# define OSAL_SYSTEM_QT
#endif

#if defined (_WIN32)
# define OSAL_SYSTEM_WINDOWS
#endif

#if !defined (OSAL_SYSTEM_WINDOWS)
# define OSAL_SYSTEM_POSIX
#endif

#if defined (OSAL_SYSTEM_IOS) || defined (OSAL_SYSTEM_ANDROID)
# define OSAL_SYSTEM_MOBILE
#endif

/** Architecture macros */

#if defined (OSAL_COMPILER_GCC)
# if defined (__arm__) || defined (__thumb__) || defined (__thumb2__)
#  define OSAL_ARCH_ARM32
# endif
# if defined (__aarch64__)
#  define OSAL_ARCH_ARM64
# endif
# if defined (i386) || defined (__i386) || defined (__i386__)
#  define OSAL_ARCH_X86_32
# endif
# if defined (__amd64__) || defined(__amd64) || defined (__x86_64__) || defined (__x86_64)
#  define OSAL_ARCH_X86_64
# endif
#endif

#if defined (OSAL_COMPILER_MSVC)
# if defined (_M_IX86)
#  define OSAL_ARCH_X86_32
# endif
# if defined (_M_X64) || defined (_M_AMD64)
#  define OSAL_ARCH_X86_64
# endif
#endif

#if defined (OSAL_ARCH_ARM32) || defined (OSAL_ARCH_ARM64)
# define OSAL_ARCH_ARM
#endif

#if defined (OSAL_ARCH_X86_32) || defined (OSAL_ARCH_X86_64)
# define OSAL_ARCH_X86
#endif

#if defined (OSAL_ARCH_ARM32) || defined (OSAL_ARCH_X86_32)
# define OSAL_ARCH_32BIT
#endif

#if defined (OSAL_ARCH_ARM64) || defined (OSAL_ARCH_X86_64)
# define OSAL_ARCH_64BIT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (OSAL_SYSTEM_NACL)
	void osal_pepper_init(int a_module_id, const void*(*get_browser_interface)(const char*));
#endif

#ifdef __cplusplus
}
#endif

#endif//OSAL__H
