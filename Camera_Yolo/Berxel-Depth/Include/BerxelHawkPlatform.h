#ifndef __BERXEL_HAWK_PLATFORM_H__
#define __BERXEL_HAWK_PLATFORM_H__

#ifndef BERXEL_HAWK_API_EXPORT

#if defined(_WIN32)
#define BERXEL_HAWK_API_EXPORT __declspec(dllexport)
#elif defined (ANDROID) || defined (__linux__)
#define BERXEL_HAWK_API_EXPORT __attribute__ ((visibility("default")))
#else
#	Unsupported Platform!
#endif

#endif

#include <stddef.h>

#endif