/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/

#ifndef __HTTP_SESSION_H__
#define __HTTP_SESSION_H__

#include "HTTPSessionInterface.h"

#include "HTTPRequest.h"
#include "TimeoutTask.h"
#include "QTSSModule.h"
#include "OSQueue.h"

using namespace std;

class HTTPSession : public HTTPSessionInterface
{
public:
	HTTPSession();
	virtual ~HTTPSession();

	QTSS_Error SendHTTPPacket(const string& msg, bool connectionClose, bool decrement);

	string GetTalkbackSession() const { return talkbackSession; }
	void SetTalkbackSession(const string& session) { talkbackSession = session; }

	string GetDarwinHTTPPort() const { return darwinHttpPort_; }

private:
	SInt64 Run() override;

	// Does request prep & request cleanup, respectively
	QTSS_Error setupRequest();
	void cleanupRequest();
	bool isRightChannel(const char* channel) const;

	QTSS_Error processRequest();
	QTSS_Error execNetMsgErrorReqHandler(HTTPStatusCode errCode);
	QTSS_Error execNetMsgDSRegisterReq(const char* json);
	QTSS_Error execNetMsgDSPushStreamAck(const char* json) const;
	QTSS_Error execNetMsgCSFreeStreamReq(const char *json);
	QTSS_Error execNetMsgDSStreamStopAck(const char* json) const;
	QTSS_Error execNetMsgDSPostSnapReq(const char* json);
	static QTSS_Error execNetMsgDSPTZControlAck(const char* json);
	QTSS_Error execNetMsgDSPresetControlAck(const char* json) const;

	QTSS_Error execNetMsgCSTalkbackControlReq(const char* json);
	static QTSS_Error execNetMSGDSTalkbackControlAck(const char* json);

	QTSS_Error execNetMsgCSDeviceListReq(const char* json);
	QTSS_Error execNetMsgCSCameraListReq(const char* json);

	QTSS_Error execNetMsgCSStartStreamReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSStopStreamReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSGetDeviceListReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSGetCameraListReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSPTZControlReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSPresetControlReqRESTful(const char* queryString);

	QTSS_Error execNetMsgCSGetBaseConfigReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSSetBaseConfigReqRESTful(const char* queryString);

	static QTSS_Error execNetMsgCSRestartReqRESTful(const char* queryString);

	QTSS_Error execNetMsgCSGetUsagesReqRESTful(const char* queryString);

	QTSS_Error dumpRequestData();

	// test current connections handled by this object against server pref connection limit
	static bool overMaxConnections(UInt32 buffer);

	void addDevice() const;

	HTTPRequest* fRequest;
	OSMutex fReadMutex;
	OSMutex fSendMutex;

	enum class State
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

	State state_;

	QTSS_ModuleState fModuleState;

	string talkbackSession;

	string darwinHttpPort_;

};

#endif // __HTTP_SESSION_H__

