#ifndef __CREATE_DUMP_H__
#define __CREATE_DUMP_H__

#ifdef _WIN32
#include <winsock2.h>
#include <DbgHelp.h>
#pragma comment(lib,"DbgHelp.lib")




void CreateDumpFile(LPCSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException);






#endif



#endif
