#ifndef MCX_API_DATA_DEFINITIONS_H
#define MCX_API_DATA_DEFINITIONS_H

/**
 * Defines for API readablity
 */
#define IN
#define OUT

#if defined(_WIN32)
#define CALLCLIENT __cdecl
#if defined(MCX_LIB_STATIC)
#define MCX_API
#elif defined(MCX_LIB_BUILDING_LIBRARY)
#define MCX_API __declspec(dllexport)
#else
#define MCX_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define CALLCLIENT
#if defined(MCX_LIB_STATIC)
#define MCX_API
#else
#define MCX_API __attribute__((visibility("default")))
#endif
#else
#define CALLCLIENT
#define MCX_API
#endif

#endif
