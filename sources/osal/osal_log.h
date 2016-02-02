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

/** Simple log wrappers */

#ifndef OSAL_LOG__H
#define OSAL_LOG__H

#include "osal.h"

#ifndef OSAL_LOG_TAG
# define OSAL_LOG_TAG	""
#endif

#ifdef OSAL_SYSTEM_POSIX
# ifdef OSAL_SYSTEM_DARWIN
#  define OSAL_COLOR_RED	"\xE2\x9D\x8C"
#  define OSAL_COLOR_GREEN	"\xE2\x9C\x85"
#  define OSAL_COLOR_YELLOW
#  define OSAL_COLOR_BLUE
#  define OSAL_COLOR_MAGENTA
#  define OSAL_COLOR_CYAN
#  define OSAL_COLOR_RESET
# else
#  define OSAL_COLOR_RED     "\x1b[31m"
#  define OSAL_COLOR_GREEN   "\x1b[32m"
#  define OSAL_COLOR_YELLOW  "\x1b[33m"
#  define OSAL_COLOR_BLUE    "\x1b[34m"
#  define OSAL_COLOR_MAGENTA "\x1b[35m"
#  define OSAL_COLOR_CYAN    "\x1b[36m"
#  define OSAL_COLOR_RESET   "\x1b[0m"
# endif
#endif

#if defined (OSAL_SYSTEM_WINDOWS) && defined (OSAL_COMPILER_MSVC)
# include <stdio.h>
# include <Windows.h>
# define OSAL_COLOR_BLUE	FOREGROUND_BLUE
# define OSAL_COLOR_GREEN	FOREGROUND_GREEN
# define OSAL_COLOR_RED		FOREGROUND_RED
# define OSAL_COLOR_YELLOW	(FOREGROUND_RED | FOREGROUND_GREEN)
# define OSAL_COLOR_MAGENTA	(FOREGROUND_BLUE | FOREGROUND_RED)
# define OSAL_COLOR_CYAN	(FOREGROUND_BLUE | FOREGROUND_GREEN)
# define OSAL_LOG(color, fmt, ...) do { \
	printf("(V): " OSAL_LOG_TAG ": %s: " ,__FUNCTION__); \
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo; \
	GetConsoleScreenBufferInfo(hConsole, &consoleInfo); \
	SetConsoleTextAttribute(hConsole, color); \
	printf(fmt, ## __VA_ARGS__); \
	SetConsoleTextAttribute(hConsole, consoleInfo.wAttributes); \
} while (0)
# define LOGV(fmt, ...) ((void)printf("(V): " OSAL_LOG_TAG ": %s: " fmt, __FUNCTION__ , ## __VA_ARGS__))
# define LOGD(fmt, ...) ((void)printf("(D): " OSAL_LOG_TAG ": %s: " fmt, __FUNCTION__ , ## __VA_ARGS__))
# define LOGI(fmt, ...) ((void)printf("(I): " OSAL_LOG_TAG ": %s: " fmt, __FUNCTION__ , ## __VA_ARGS__))
# define LOGW(fmt, ...) ((void)printf("(W): " OSAL_LOG_TAG ": %s: " fmt, __FUNCTION__ , ## __VA_ARGS__))
# define LOGE(fmt, ...) ((void)printf("(E): " OSAL_LOG_TAG ": %s: " fmt, __FUNCTION__ , ## __VA_ARGS__))
#elif defined (OSAL_SYSTEM_ANDROID)
# include <android/log.h>
# define LOGV(fmt, ...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, OSAL_LOG_TAG,  "%s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGD(fmt, ...) ((void)__android_log_print(ANDROID_LOG_DEBUG, OSAL_LOG_TAG,  "%s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGI(fmt, ...) ((void)__android_log_print(ANDROID_LOG_INFO, OSAL_LOG_TAG,  "%s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGW(fmt, ...) ((void)__android_log_print(ANDROID_LOG_WARN, OSAL_LOG_TAG,  "%s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGE(fmt, ...) ((void)__android_log_print(ANDROID_LOG_ERROR, OSAL_LOG_TAG, "%s: " fmt, __func__ , ## __VA_ARGS__))
#elif defined (OSAL_SYSTEM_DARWIN)
# include <stdio.h>
# include <sys/time.h>
# include <time.h>
# include <pthread.h>
# include <unistd.h>
# define OSAL_LOG(color, fmt, ...) do { 																			\
	struct timeval tv;																								\
	struct tm * lt;																									\
	gettimeofday(&tv, NULL);																						\
	lt = localtime(&tv.tv_sec);																						\
	fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d.%06d %6d (L): " OSAL_LOG_TAG ": %s: " color fmt OSAL_COLOR_RESET, lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec, pthread_mach_thread_np(pthread_self()), __func__, ## __VA_ARGS__);\
} while (0)
# define LOGE(fmt, ...)																								\
do {																												\
	struct timeval tv;																								\
	struct tm * lt;																									\
	gettimeofday(&tv, NULL);																						\
	lt = localtime(&tv.tv_sec);																						\
	fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d.%06d %6d (E): " OSAL_LOG_TAG ": %s: " fmt, lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec, pthread_mach_thread_np(pthread_self()), __func__, ## __VA_ARGS__);\
} while (0)

# define LOGW(fmt, ...)																								\
do {																												\
	struct timeval tv;																								\
	struct tm * lt;																									\
	gettimeofday(&tv, NULL);																						\
	lt = localtime(&tv.tv_sec);																						\
	fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d.%06d %6d (W): " OSAL_LOG_TAG ": %s: " fmt, lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec, pthread_mach_thread_np(pthread_self()), __func__, ## __VA_ARGS__);\
} while (0)

# define LOGI(fmt, ...)																								\
do {																												\
	struct timeval tv;																								\
	struct tm * lt;																									\
	gettimeofday(&tv, NULL);																						\
	lt = localtime(&tv.tv_sec);																						\
	fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d.%06d %6d (I): " OSAL_LOG_TAG ": %s: " fmt, lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec, pthread_mach_thread_np(pthread_self()), __func__, ## __VA_ARGS__);\
} while (0)

# define LOGD(fmt, ...)																								\
do {																												\
	struct timeval tv;																								\
	struct tm * lt;																									\
	gettimeofday(&tv, NULL);																						\
	lt = localtime(&tv.tv_sec);																						\
	fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d.%06d %6d (D): " OSAL_LOG_TAG ": %s: " fmt, lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec, pthread_mach_thread_np(pthread_self()), __func__, ## __VA_ARGS__);\
} while (0)

# define LOGV(fmt, ...)																								\
do {																												\
	struct timeval tv;																								\
	struct tm * lt;																									\
	gettimeofday(&tv, NULL);																						\
	lt = localtime(&tv.tv_sec);																						\
	fprintf(stderr, "%d-%02d-%02d %02d:%02d:%02d.%06d %6d (V): " OSAL_LOG_TAG ": %s: " fmt, lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec, pthread_mach_thread_np(pthread_self()), __func__, ## __VA_ARGS__);\
} while (0)
#elif defined (OSAL_SYSTEM_NACL)
# include <stdio.h>
# define LOGV(fmt, ...) ((void)fprintf(stderr, "(V): " OSAL_LOG_TAG ": %s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGD(fmt, ...) ((void)fprintf(stderr, "(D): " OSAL_LOG_TAG ": %s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGI(fmt, ...) ((void)fprintf(stderr, "(I): " OSAL_LOG_TAG ": %s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGW(fmt, ...) ((void)fprintf(stderr, "(W): " OSAL_LOG_TAG ": %s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGE(fmt, ...) ((void)fprintf(stderr, "(E): " OSAL_LOG_TAG ": %s: " fmt, __func__ , ## __VA_ARGS__))
#else
# include <stdio.h>
# define LOGV(fmt, ...) ((void)printf("(V): " OSAL_LOG_TAG ": %s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGD(fmt, ...) ((void)printf("(D): " OSAL_LOG_TAG ": %s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGI(fmt, ...) ((void)printf("(I): " OSAL_LOG_TAG ": %s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGW(fmt, ...) ((void)printf("(W): " OSAL_LOG_TAG ": %s: " fmt, __func__ , ## __VA_ARGS__))
# define LOGE(fmt, ...) ((void)printf("(E): " OSAL_LOG_TAG ": %s: " fmt, __func__ , ## __VA_ARGS__))
#endif



#if defined(OSAL_HIDE_LOGS)
# undef LOGV
# undef LOGD
# undef LOGI
# undef LOGW
# undef LOGE
#endif

#ifdef OSAL_HIDE_LOGV
# undef LOGV
#endif

#ifdef OSAL_HIDE_LOGD
# undef LOGD
#endif

#ifdef OSAL_HIDE_LOGI
# undef LOGI
#endif

#ifdef OSAL_HIDE_LOGW
# undef LOGW
#endif

#ifdef OSAL_HIDE_LOGE
# undef LOGE
#endif


#ifndef LOGV
# define LOGV(...)
#endif

#ifndef LOGD
# define LOGD(...)
#endif

#ifndef LOGI
# define LOGI(...)
#endif

#ifndef LOGW
# define LOGW(...)
#endif

#ifndef LOGE
# define LOGE(...)
#endif

#endif//OSAL_LOG__H
