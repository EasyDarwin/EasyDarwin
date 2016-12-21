#ifdef _WIN32

#include "CreateDump.h"


//创建dump文件
void CreateDumpFile(LPCSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
	//创建Dump文件

	HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	//Dump信息
	MINIDUMP_EXCEPTION_INFORMATION	dumpInfo;
	dumpInfo.ExceptionPointers	=	pException;
	dumpInfo.ThreadId			=	GetCurrentThreadId();
	dumpInfo.ClientPointers		=	TRUE;

	//写入Dump文件内容
	//MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpWithFullMemory, &dumpInfo, NULL, NULL);

	CloseHandle(hDumpFile);

	/*
	wchar_t wszErr[512] = {0,};
	wsprintf(wszErr, TEXT("日志文件: %s\n异常代码: %0x%8.8X\n异常标志\n异常地址: %0x%8.8X\n\n请将该文件发给软件作者."),
		lpstrDumpFilePathName, 
		pException->ExceptionRecord->ExceptionCode, 
		pException->ExceptionRecord->ExceptionFlags,
		pException->ExceptionRecord->ExceptionAddress);
*/
	//FatalAppExit(-1, wszErr);
}


#endif
