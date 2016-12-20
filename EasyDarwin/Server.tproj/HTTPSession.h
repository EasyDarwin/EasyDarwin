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
	QTSS_Error SendHTTPPacket(StrPtrLen* contentXML, bool connectionClose, bool decrement) override;

private:
	SInt64 Run() override;

	// Does request prep & request cleanup, respectively
	QTSS_Error SetupRequest();
	void CleanupRequest();

	QTSS_Error execNetMsgCSGetServerVersionReqRESTful(const char* queryString);

	QTSS_Error execNetMsgCSLoginReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSLogoutReqRESTful(const char* queryString);

	QTSS_Error execNetMsgCSGetBaseConfigReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSSetBaseConfigReqRESTful(const char* queryString);

	QTSS_Error execNetMsgCSGetDeviceStreamReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSLiveDeviceStreamReqRESTful(const char* queryString);

	QTSS_Error execNetMsgCSGetRTSPLiveSessionsRESTful(const char* queryString);

	QTSS_Error execNetMsgCSGetRTSPRecordSessionsRESTful(const char* queryString);
	QTSS_Error execNetMsgCSRestartServiceRESTful(const char* queryString);

	//QTSS_Error ExecNetMsgEasyHLSModuleReq(char* queryString, char* json);
	//QTSS_Error ExecNetMsgGetHlsSessionsReq(char* queryString, char* json);

	// test current connections handled by this object against server pref connection limit
	static inline bool OverMaxConnections(UInt32 buffer);

	QTSS_Error dumpRequestData();

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
};
#endif // __HTTPSESSION_H__

