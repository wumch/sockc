
#pragma once

#include "stage/meta.hpp"

#define _CSOCKS_POSIX       1
#define _CSOCKS_WIN         2
#define _CSOCKS_EMBED       3

#define _CSOCKS_LINUX       11
#define _CSOCKS_WINDOWS_PC  12
#define _CSOCKS_OSX         13

#define _CSOCKS_ANDROID         101
#define _CSOCKS_WINDOWS_PHONE   102
#define _CSOCKS_IOS             103

// follows are in planning:
#define _CSOCKS_OPENWRT         201
#define _CSOCKS_DDWRT           202
#define _CSOCKS_COMWARE         203
#define _CSOCKS_CISCO_IOS       204

#ifdef __linux__
#   define _CSOCKS_PLATFORM _CSOCKS_LINUX
#elif defined(__WIN32__)
#   define _CSOCKS_PLATFORM _CSOCKS_WINDOWS_PC
#elif defined(__OSX__)
#   define _CSOCKS_PLATFORM _CSOCKS_OSX
#elif defined(__ANDROID__)
#   define _CSOCKS_PLATFORM _CSOCKS_ANDROID
#elif defined(WINAPI) and defined(WINAPI_FAMILY_PHONE_APP) and \
    (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#   define _CSOCKS_PLATFORM _CSOCKS_WINDOWS_PHONE
#elif defined(TARGET_OS_IPHONE)
#   define _CSOCKS_PLATFORM _CSOCKS_IOS
#endif

#define _CSOCKS_IS_LINUX            (_CSOCKS_PLATFORM == _CSOCKS_LINUX)
#define _CSOCKS_IS_WINDOWS_PC       (_CSOCKS_PLATFORM == _CSOCKS_WINDOWS_PC)
#define _CSOCKS_IS_OSX              (_CSOCKS_PLATFORM == _CSOCKS_OSX)
#define _CSOCKS_IS_ANDROID          (_CSOCKS_PLATFORM == _CSOCKS_ANDROID)
#define _CSOCKS_IS_WINDOWS_PHONE    (_CSOCKS_PLATFORM == _CSOCKS_WINDOWS_PHONE)
#define _CSOCKS_IS_IOS              (_CSOCKS_PLATFORM == _CSOCKS_IOS)

#define _CSOCKS_IS_POSIX            (_CSOCKS_IS_LINUX or _CSOCKS_IS_OSX or _CSOCKS_IS_ANDROID or _CSOCKS_IS_IOS)
#define _CSOCKS_IS_WINDOWS          (_CSOCKS_IS_WINDOWS_PC or _CSOCKS_IS_WINDOWS_PHONE)
