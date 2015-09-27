#pragma once

#if defined(__GNUC__) //gnu gcc/g++
#include <stdint.h>
#include <inttypes.h>
#endif // __GNUC__

#ifndef _STDINT_H  
#define _STDINT_H
#if defined(_MSC_VER) //microsoft vc studio 
#if defined(WIN32) || defined(_WIN32)

	typedef char				int8_t;
	typedef unsigned char		uint8_t;
	typedef short				int16_t;
	typedef unsigned short		uint16_t;
	typedef int					int32_t;
	typedef unsigned int		uint32_t;
#ifndef _WIN32_WCE
	typedef long long			int64_t;
	typedef unsigned long long	uint64_t;
#endif
#else if defined(_WIN64)
	typedef char				int8_t;
	typedef unsigned char		uint8_t;
	typedef short				int16_t;
	typedef unsigned short		uint16_t;
	typedef int					int32_t;
	typedef unsigned int		uint32_t;
	typedef long 				int64_t;
	typedef unsigned long		uint64_t;
#endif
#endif	//_MSC_VER
#endif

enum
{
	success = 0,
	fail = 1,
};

#ifndef NULL
	#define NULL 0
#endif

#ifndef HANDLE
	typedef void * HANDLE;
#endif

#ifndef LPVOID
	typedef void * LPVOID;
#endif

