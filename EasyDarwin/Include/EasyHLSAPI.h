/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _Easy_HLS_API_H
#define _Easy_HLS_API_H

#ifdef _WIN32
#define EasyHLS_API  __declspec(dllexport)
#define Easy_APICALL  __stdcall
#else
#define EasyHLS_API
#define Easy_APICALL 
#endif

#define Easy_HLS_Handle void*

enum{
	TS_TYPE_PAT = 0x01000000,
	TS_TYPE_PMT = 0x02000000,
	TS_TYPE_PES = 0x03f00000,

	TS_TYPE_PES_AUDIO         = 0x03100000,
	TS_TYPE_PES_VIDEO_I_FRAME = 0x03200000,
	TS_TYPE_PES_VIDEO_P_FRAME = 0x03400000,
	TS_TYPE_PES_VIDEO_E_FRAME = 0x03800000,
};


#ifdef __cplusplus
extern "C"
{
#endif

	EasyHLS_API Easy_HLS_Handle Easy_APICALL EasyHLS_Session_Create(int nCapacity, bool bAllowCache, int version);

	EasyHLS_API void Easy_APICALL EasyHLS_ResetStreamCache(Easy_HLS_Handle handle, const char * strRootDir, const char* strSubDir, const char* strMediaName, int nTargetDuration);
	
	EasyHLS_API const char*  Easy_APICALL EasyHLS_GetM3U8File(Easy_HLS_Handle handle);
	
	EasyHLS_API int Easy_APICALL EasyHLS_VideoMux(Easy_HLS_Handle handle, unsigned int uiFrameType, unsigned char *data, int dataLength, unsigned long long pcr, unsigned long long pts, unsigned long long dts);
	
	EasyHLS_API int Easy_APICALL EasyHLS_AudioMux(Easy_HLS_Handle handle, unsigned char *data, int dataLength, /*u64 pcr,*/ unsigned long long pts, unsigned long long dts=~0);

	EasyHLS_API void Easy_APICALL EasyHLS_Session_Release(Easy_HLS_Handle handle);
#ifdef __cplusplus
}
#endif

#endif
