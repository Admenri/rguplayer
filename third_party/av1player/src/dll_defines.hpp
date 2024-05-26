#ifndef _UVPX_DLL_DEFINES_H_
#define _UVPX_DLL_DEFINES_H_

#define UVPX_EXPORT

#if defined(__CYGWIN32__)
#define UVPX_INTERFACE_API __stdcall
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || \
    defined(_WIN64) || defined(WINAPI_FAMILY)
#define UVPX_INTERFACE_API __stdcall
#elif defined(__MACH__) || defined(__ANDROID__) || defined(__linux__) || \
    defined(__QNX__)
#define UVPX_INTERFACE_API
#else
#define UVPX_INTERFACE_API
#endif

#endif  // _UVPX_DLL_DEFINES_H_
