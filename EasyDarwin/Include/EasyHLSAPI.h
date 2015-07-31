#ifndef _Easy_HLS_API_H
#define _Easy_HLS_API_H

#ifdef _WIN32
#define EasyHLS_API  __declspec(dllexport)
#define EasyHLS_APICALL  __stdcall
#else
#define EasyHLS_API
#define EasyHLS_APICALL 
#endif

#define Easy_HLS_Handle void*

#ifdef __cplusplus
extern "C"
{
#endif

	EasyHLS_API Easy_HLS_Handle EasyHLS_APICALL NewManagerHandle(int nCapacity, bool bAllowCache, int version);

	EasyHLS_API void EasyHLS_APICALL ResetStreamCache(Easy_HLS_Handle handle, const char * strRootDir, const char* strSubDir, const char* strMediaName, int nTargetDuration);
	
	EasyHLS_API const char*  EasyHLS_APICALL GetM3U8File(Easy_HLS_Handle handle);
	
	EasyHLS_API int EasyHLS_APICALL VideoMux(Easy_HLS_Handle handle, unsigned int uiFrameType, unsigned char *data, int dataLength, unsigned long long pcr, unsigned long long pts, unsigned long long dts);
	
	EasyHLS_API int EasyHLS_APICALL AudioMux(Easy_HLS_Handle handle, unsigned char *data, int dataLength, /*u64 pcr,*/ unsigned long long pts, unsigned long long dts=~0);

#ifdef __cplusplus
}
#endif

#endif
