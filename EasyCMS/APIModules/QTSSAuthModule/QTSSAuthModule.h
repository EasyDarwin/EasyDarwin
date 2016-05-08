#ifndef __QTSSAUTHMODULE__H__
#define __QTSSAUTHMODULE__H__

#include "QTSS.h"

extern "C"
{
	EXPORT QTSS_Error QTSSAuthModule_Main(void* inPrivateArgs);
}

#endif 