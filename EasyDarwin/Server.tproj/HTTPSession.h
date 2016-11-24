/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/

#ifndef __HTTPSESSION_H__
#define __HTTPSESSION_H__

#include "HTTPSessionInterface.h"
#include "HTTPRequest.h"
#include "QTSSModule.h"

class HTTPSession : public HTTPSessionInterface
{
public:
	HTTPSession();
	virtual ~HTTPSession();

	//Send HTTPPacket
	virtual QTSS_Error SendHTTPPacket(StrPtrLen* contentXML, bool connectionClose, bool decrement);

private:
	SInt64 Run();

	// Does request prep & request cleanup, respectively
	QTSS_Error SetupRequest();
	void CleanupRequest();

	QTSS_Error execNetMsgCSLoginReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSLogoutReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSGetServerVersionReqRESTful(const char* queryString);
	QTSS_Error ExecNetMsgCSGetRTSPLiveSessionsRESTful(char* queryString, char* json);
	QTSS_Error execNetMsgCSRestartServiceRESTful(const char* queryString) const;


	QTSS_Error ExecNetMsgEasyHLSModuleReq(char* queryString, char* json);
	QTSS_Error ExecNetMsgGetHlsSessionsReq(char* queryString, char* json);

	// test current connections handled by this object against server pref connection limit
	bool OverMaxConnections(UInt32 buffer);

	HTTPRequest*        fRequest;
	OSMutex             fReadMutex;

	enum
	{
		kReadingRequest = 0,
		kFilteringRequest = 1,
		kPreprocessingRequest = 2,
		kProcessingRequest = 3,
		kSendingResponse = 4,
		kCleaningUp = 5,

		kReadingFirstRequest = 6,
		kHaveCompleteMessage = 7
	};

	UInt32 fCurrentModule;
	UInt32 fState;

	QTSS_RoleParams     fRoleParams;//module param blocks for roles.
	QTSS_ModuleState    fModuleState;

	QTSS_Error DumpRequestData();

};
#endif // __HTTPSESSION_H__

