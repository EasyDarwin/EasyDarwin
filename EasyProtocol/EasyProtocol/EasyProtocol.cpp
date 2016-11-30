/*
    Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
    Github: https://github.com/EasyDarwin
    WEChat: EasyDarwin
    Website: http://www.easydarwin.org
*/

#include <EasyProtocol.h>
#include <EasyUtil.h>

namespace EasyDarwin { namespace Protocol
{
	EasyDevice::EasyDevice()
	{
		snapJpgPath_.clear();
	}

	EasyDevice::EasyDevice(const string& channel, const string& name)
		: serial_(channel)
		, name_(name)
	{

	}

	EasyDevice::EasyDevice(const string& channel, const string& name, const string& status)
		: serial_(channel)
		, name_(name)
		, status_(status)
	{
	}


	EasyNVR::EasyNVR() : object_(NULL)
	{
	}

	EasyNVR::EasyNVR(const string& serial, const string& name, const string& password, const string& tag, EasyDevices &channel)
		: channels_(channel), object_(NULL)
	{
		serial_ = serial;
		name_ = name;
		password_ = password;
		tag_ = tag;
	}

	// MSG_DS_REGISTER_REQ��Ϣ���Ĺ���
	EasyMsgDSRegisterREQ::EasyMsgDSRegisterREQ(EasyDarwinTerminalType terminalType, EasyDarwinAppType appType, EasyNVR &nvr, size_t cseq)
		: EasyProtocol(MSG_DS_REGISTER_REQ)
		, nvr_(nvr)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_APP_TYPE, EasyProtocol::GetAppTypeString(appType));
		SetHeaderValue(EASY_TAG_TERMINAL_TYPE, EasyProtocol::GetTerminalTypeString(terminalType));

		SetBodyValue(EASY_TAG_SERIAL, nvr.serial_);
		SetBodyValue(EASY_TAG_NAME, nvr.name_);
		SetBodyValue(EASY_TAG_TAG, nvr.tag_);
		SetBodyValue(EASY_TAG_TOKEN, nvr.password_);

		if (appType == EASY_APP_TYPE_NVR)
		{
			SetBodyValue(EASY_TAG_CHANNEL_COUNT, nvr.channels_.size());
			for (EasyDevices::iterator it = nvr.channels_.begin(); it != nvr.channels_.end(); ++it)
			{
				Json::Value value;
				value[EASY_TAG_CHANNEL] = it->second.channel_;
				value[EASY_TAG_NAME] = it->second.name_;
				value[EASY_TAG_STATUS] = it->second.status_;
				root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS].append(value);
			}
		}
	}

	// MSG_DS_REGISTER_REQ��Ϣ���Ľ���
	EasyMsgDSRegisterREQ::EasyMsgDSRegisterREQ(const string& msg)
		: EasyProtocol(msg, MSG_DS_REGISTER_REQ)
	{
		nvr_.serial_ = GetBodyValue(EASY_TAG_SERIAL);
		nvr_.name_ = GetBodyValue(EASY_TAG_NAME);
		nvr_.tag_ = GetBodyValue(EASY_TAG_TAG);
		nvr_.password_ = GetBodyValue(EASY_TAG_TOKEN);

		nvr_.channels_.clear();

		int size = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS].size();
		for (int i = 0; i < size; i++)
		{
			Json::Value &json_camera = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS][i];
			EasyDevice channel;
			channel.channel_ = json_camera[EASY_TAG_CHANNEL].asString();
			channel.name_ = json_camera[EASY_TAG_NAME].asString();
			channel.status_ = json_camera[EASY_TAG_STATUS].asString();
			//nvr_.channels_.push_back(channel);
			nvr_.channels_[channel.channel_] = channel;
		}
	}

	// MSG_SD_REGISTER_ACK��Ϣ����
	EasyMsgSDRegisterACK::EasyMsgSDRegisterACK(EasyJsonValue &body, size_t cseq, size_t error)
		: EasyProtocol(MSG_SD_REGISTER_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	// MSG_SD_REGISTER_ACK��Ϣ����
	EasyMsgSDRegisterACK::EasyMsgSDRegisterACK(const string& msg)
		: EasyProtocol(msg, MSG_SD_REGISTER_ACK)
	{
	}

	// MSG_SD_PUSH_STREAM_REQ��Ϣ����
	EasyMsgSDPushStreamREQ::EasyMsgSDPushStreamREQ(EasyJsonValue &body, size_t cseq)
		: EasyProtocol(MSG_SD_PUSH_STREAM_REQ)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	// MSG_SD_PUSH_STREAM_REQ��Ϣ����
	EasyMsgSDPushStreamREQ::EasyMsgSDPushStreamREQ(const string& msg)
		: EasyProtocol(msg, MSG_SD_PUSH_STREAM_REQ)
	{
	}

	// MSG_DS_PUSH_STREAM_ACK��Ϣ����
	EasyMsgDSPushSteamACK::EasyMsgDSPushSteamACK(EasyJsonValue &body, size_t cseq, size_t error)
		: EasyProtocol(MSG_DS_PUSH_STREAM_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	// MSG_CS_FREE_STREAM_REQ��Ϣ����
	EasyMsgCSFreeStreamREQ::EasyMsgCSFreeStreamREQ(EasyJsonValue &body, size_t cseq)
		: EasyProtocol(MSG_CS_FREE_STREAM_REQ)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	// MSG_CS_FREE_STREAM_REQ��Ϣ����
	EasyMsgCSFreeStreamREQ::EasyMsgCSFreeStreamREQ(const string& msg)
		: EasyProtocol(msg, MSG_CS_FREE_STREAM_REQ)
	{
	}

	// MSG_DS_PUSH_STREAM_ACK��Ϣ����
	EasyMsgDSPushSteamACK::EasyMsgDSPushSteamACK(const string& msg)
		: EasyProtocol(msg, MSG_DS_PUSH_STREAM_ACK)
	{
	}

	// MSG_SD_STREAM_STOP_REQ��Ϣ����
	EasyMsgSDStopStreamREQ::EasyMsgSDStopStreamREQ(EasyJsonValue& body, size_t cseq)
		: EasyProtocol(MSG_SD_STREAM_STOP_REQ)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	// MSG_SD_STREAM_STOP_REQ��Ϣ����
	EasyMsgSDStopStreamREQ::EasyMsgSDStopStreamREQ(const string& msg)
		: EasyProtocol(msg, MSG_SD_STREAM_STOP_REQ)
	{
	}

	// MSG_SD_STREAM_STOP_REQ��Ϣ����
	EasyMsgDSStopStreamACK::EasyMsgDSStopStreamACK(EasyJsonValue &body, size_t cseq, size_t error)
		: EasyProtocol(MSG_DS_STREAM_STOP_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	// MSG_SD_STREAM_STOP_REQ��Ϣ����
	EasyMsgDSStopStreamACK::EasyMsgDSStopStreamACK(const string& msg)
		: EasyProtocol(msg, MSG_SD_STREAM_STOP_REQ)
	{
	}

	EasyMsgSCStartHLSACK::EasyMsgSCStartHLSACK()
		: EasyProtocol(MSG_SC_START_HLS_ACK)
	{
	}

	EasyMsgSCStartHLSACK::EasyMsgSCStartHLSACK(const string& msg)
		: EasyProtocol(msg, MSG_SC_START_HLS_ACK)
	{
	}

	void EasyMsgSCStartHLSACK::SetStreamName(const string& sName)
	{
		SetBodyValue(EASY_TAG_L_NAME, sName);
	}

	void EasyMsgSCStartHLSACK::SetStreamURL(const string& sURL)
	{
		SetBodyValue(EASY_TAG_L_URL, sURL);
	}

	EasyMsgSCHLSessionListACK::EasyMsgSCHLSessionListACK()
		: EasyProtocol(MSG_SC_HLS_SESSION_LIST_ACK)
	{
	}

	EasyMsgSCHLSessionListACK::EasyMsgSCHLSessionListACK(const string& msg)
		: EasyProtocol(msg, MSG_SC_HLS_SESSION_LIST_ACK)
	{
	}

	bool EasyMsgSCHLSessionListACK::AddSession(EasyDarwinHLSession &session)
	{
		Json::Value value;
		value[EASY_TAG_L_INDEX] = session.index;
		value[EASY_TAG_L_NAME] = session.SessionName;
		value[EASY_TAG_L_URL] = session.HlsUrl;
		value[EASY_TAG_L_SOURCE] = session.sourceUrl;
		value[EASY_TAG_BITRATE] = session.bitrate;
		root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_SESSIONS].append(value);
		return true;
	}


	EasyMsgSCDeviceListACK::EasyMsgSCDeviceListACK(EasyDevices & devices, size_t cseq, size_t error)
		: EasyProtocol(MSG_SC_DEVICE_LIST_ACK)
		, devices_(devices)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		SetBodyValue(EASY_TAG_DEVICE_COUNT, devices.size());
		for (EasyDevices::iterator it = devices.begin(); it != devices.end(); ++it)
		{
			Json::Value value;
			value[EASY_TAG_SERIAL] = it->second.serial_;
			value[EASY_TAG_NAME] = it->second.name_;
			value[EASY_TAG_TAG] = it->second.tag_;
			//value["Status"] = it->status_;
			root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_DEVICES].append(value);
		}
	}

	EasyMsgSCDeviceListACK::EasyMsgSCDeviceListACK(const string& msg)
		: EasyProtocol(msg, MSG_SC_DEVICE_LIST_ACK)
	{
		devices_.clear();
		int size = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_DEVICES].size();

		for (int i = 0; i < size; i++)
		{
			Json::Value &json_ = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_DEVICES][i];
			EasyDevice device;
			device.name_ = json_[EASY_TAG_NAME].asString();
			device.serial_ = json_[EASY_TAG_SERIAL].asString();
			device.tag_ = json_[EASY_TAG_TAG].asString();
			//device.status_ = json_["Status"].asString();
			devices_[device.serial_] = device;
		}
	}

	EasyMsgSCDeviceInfoACK::EasyMsgSCDeviceInfoACK(EasyDevices& cameras, const string& device_serial, size_t cseq, size_t error)
		: EasyProtocol(MSG_SC_CAMERA_LIST_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		SetBodyValue(EASY_TAG_SERIAL, device_serial);
		SetBodyValue(EASY_TAG_CHANNEL_COUNT, cameras.size());
		for (EasyDevices::iterator it = cameras.begin(); it != cameras.end(); ++it)
		{
			Json::Value value;
			value[EASY_TAG_SERIAL] = it->second.serial_;
			value[EASY_TAG_NAME] = it->second.name_;
			value[EASY_TAG_STATUS] = it->second.status_;
			root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS].append(value);
		}
	}

	EasyMsgSCDeviceInfoACK::EasyMsgSCDeviceInfoACK(const string& msg)
		: EasyProtocol(msg, MSG_SC_CAMERA_LIST_ACK)
	{
		channels_.clear();
		int size = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS].size();

		for (int i = 0; i < size; i++)
		{
			Json::Value &json_ = root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS][i];
			EasyDevice channel;
			channel.name_ = json_[EASY_TAG_SERIAL].asString();
			channel.serial_ = json_[EASY_TAG_NAME].asString();
			channel.status_ = json_[EASY_TAG_STATUS].asString();
			channels_[channel.serial_] = channel;
		}
	}

	EasyMsgSCGetStreamACK::EasyMsgSCGetStreamACK(EasyJsonValue &body, size_t cseq, size_t error)
		: EasyProtocol(MSG_SC_GET_STREAM_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgSCGetStreamACK::EasyMsgSCGetStreamACK(const string& msg)
		: EasyProtocol(msg, MSG_SC_GET_STREAM_ACK)
	{
	}

	EasyMsgSCFreeStreamACK::EasyMsgSCFreeStreamACK(EasyJsonValue & body, size_t cseq, size_t error)
		: EasyProtocol(MSG_SC_FREE_STREAM_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgSCFreeStreamACK::EasyMsgSCFreeStreamACK(const string& msg)
		: EasyProtocol(msg, MSG_SC_FREE_STREAM_ACK)
	{
	}

	EasyMsgDSPostSnapREQ::EasyMsgDSPostSnapREQ(EasyJsonValue & body, size_t cseq)
		: EasyProtocol(MSG_DS_POST_SNAP_REQ)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgDSPostSnapREQ::EasyMsgDSPostSnapREQ(const string& msg)
		: EasyProtocol(msg, MSG_DS_POST_SNAP_REQ)
	{
	}

	EasyMsgSDPostSnapACK::EasyMsgSDPostSnapACK(EasyJsonValue & body, size_t cseq, size_t error)
		: EasyProtocol(MSG_SD_POST_SNAP_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgSDPostSnapACK::EasyMsgSDPostSnapACK(const string& msg)
		: EasyProtocol(msg, MSG_SD_POST_SNAP_ACK)
	{
	}

	EasyMsgCSPTZControlREQ::EasyMsgCSPTZControlREQ(EasyJsonValue & body, size_t cseq)
		: EasyProtocol(MSG_CS_PTZ_CONTROL_REQ)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgCSPTZControlREQ::EasyMsgCSPTZControlREQ(const string& msg)
		: EasyProtocol(msg, MSG_CS_PTZ_CONTROL_REQ)
	{
	}

	EasyMsgSCPTZControlACK::EasyMsgSCPTZControlACK(EasyJsonValue & body, size_t cseq, size_t error)
		: EasyProtocol(MSG_SC_PTZ_CONTROL_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgSCPTZControlACK::EasyMsgSCPTZControlACK(const string& msg)
		: EasyProtocol(msg, MSG_SC_PTZ_CONTROL_ACK)
	{
	}

	EasyMsgSDControlPTZREQ::EasyMsgSDControlPTZREQ(EasyJsonValue & body, size_t cseq)
		: EasyProtocol(MSG_SD_CONTROL_PTZ_REQ)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgSDControlPTZREQ::EasyMsgSDControlPTZREQ(const string& msg)
		: EasyProtocol(msg, MSG_SD_CONTROL_PTZ_REQ)
	{
	}

	EasyMsgDSControlPTZACK::EasyMsgDSControlPTZACK(EasyJsonValue & body, size_t cseq, size_t error)
		: EasyProtocol(MSG_DS_CONTROL_PTZ_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgDSControlPTZACK::EasyMsgDSControlPTZACK(const string& msg)
		: EasyProtocol(msg, MSG_DS_CONTROL_PTZ_ACK)
	{
	}

	EasyMsgCSPresetControlREQ::EasyMsgCSPresetControlREQ(EasyJsonValue & body, size_t cseq)
		: EasyProtocol(MSG_CS_PRESET_CONTROL_REQ)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgCSPresetControlREQ::EasyMsgCSPresetControlREQ(const string& msg)
		: EasyProtocol(msg, MSG_CS_PRESET_CONTROL_REQ)
	{
	}

	EasyMsgSCPresetControlACK::EasyMsgSCPresetControlACK(EasyJsonValue & body, size_t cseq, size_t error)
		: EasyProtocol(MSG_SC_PRESET_CONTROL_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgSCPresetControlACK::EasyMsgSCPresetControlACK(const string& msg)
		: EasyProtocol(msg, MSG_SC_PRESET_CONTROL_ACK)
	{
	}

	EasyMsgSDControlPresetREQ::EasyMsgSDControlPresetREQ(EasyJsonValue & body, size_t cseq)
		: EasyProtocol(MSG_SD_CONTROL_PRESET_REQ)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgSDControlPresetREQ::EasyMsgSDControlPresetREQ(const string& msg)
		: EasyProtocol(msg, MSG_SD_CONTROL_PRESET_REQ)
	{
	}

	EasyMsgDSControlPresetACK::EasyMsgDSControlPresetACK(EasyJsonValue & body, size_t cseq, size_t error)
		: EasyProtocol(MSG_DS_CONTROL_PRESET_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgDSControlPresetACK::EasyMsgDSControlPresetACK(const string& msg)
		: EasyProtocol(msg, MSG_DS_CONTROL_PRESET_ACK)
	{
	}

	EasyMsgCSTalkbackControlREQ::EasyMsgCSTalkbackControlREQ(EasyJsonValue & body, size_t cseq)
		: EasyProtocol(MSG_CS_TALKBACK_CONTROL_REQ)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgCSTalkbackControlREQ::EasyMsgCSTalkbackControlREQ(const string& msg)
		: EasyProtocol(msg, MSG_CS_TALKBACK_CONTROL_REQ)
	{
	}

	EasyMsgSCTalkbackControlACK::EasyMsgSCTalkbackControlACK(EasyJsonValue & body, size_t cseq, size_t error)
		: EasyProtocol(MSG_SC_TALKBACK_CONTROL_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgSCTalkbackControlACK::EasyMsgSCTalkbackControlACK(const string& msg)
		: EasyProtocol(msg, MSG_SC_TALKBACK_CONTROL_ACK)
	{
	}

	EasyMsgSDControlTalkbackREQ::EasyMsgSDControlTalkbackREQ(EasyJsonValue & body, size_t cseq)
		: EasyProtocol(MSG_SD_CONTROL_TALKBACK_REQ)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgSDControlTalkbackREQ::EasyMsgSDControlTalkbackREQ(const string& msg)
		: EasyProtocol(msg, MSG_SD_CONTROL_TALKBACK_REQ)
	{
	}

	EasyMsgDSControlTalkbackACK::EasyMsgDSControlTalkbackACK(EasyJsonValue & body, size_t cseq, size_t error)
		: EasyProtocol(MSG_DS_CONTROL_TALKBACK_ACK)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));

		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	EasyMsgDSControlTalkbackACK::EasyMsgDSControlTalkbackACK(const string& msg)
		: EasyProtocol(msg, MSG_DS_CONTROL_TALKBACK_ACK)
	{
	}

	EasyMsgExceptionACK::EasyMsgExceptionACK(size_t cseq, size_t error)
		: EasyProtocol(MSG_SC_EXCEPTION)
	{
		SetHeaderValue(EASY_TAG_CSEQ, cseq);
		SetHeaderValue(EASY_TAG_ERROR_NUM, error);
		SetHeaderValue(EASY_TAG_ERROR_STRING, GetErrorString(error));
	}

	EasyMsgSCRTSPPushSessionListACK::EasyMsgSCRTSPPushSessionListACK()
		: EasyProtocol(MSG_SC_RTSP_PUSH_SESSION_LIST_ACK)
	{
	}

	EasyMsgSCRTSPPushSessionListACK::EasyMsgSCRTSPPushSessionListACK(const string& msg)
		: EasyProtocol(msg, MSG_SC_RTSP_PUSH_SESSION_LIST_ACK)
	{
	}
	bool EasyMsgSCRTSPPushSessionListACK::AddSession(EasyDarwinRTSPSession &session)
	{
		Json::Value value;
		value[EASY_TAG_L_INDEX] = session.index;
		value[EASY_TAG_L_URL] = session.Url;
		value[EASY_TAG_L_NAME] = session.Name;
		value[EASY_TAG_AUDIENCE_NUM] = session.numOutputs;
		root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_SESSIONS].append(value);
		return true;
	}

	EasyMsgSCListRecordACK::EasyMsgSCListRecordACK()
		: EasyProtocol(MSG_SC_LIST_RECORD_ACK)
	{
	}

	EasyMsgSCListRecordACK::EasyMsgSCListRecordACK(const string& msg)
		: EasyProtocol(msg, MSG_SC_LIST_RECORD_ACK)
	{
	}

	bool EasyMsgSCListRecordACK::AddRecord(const string& record)
	{
		Json::Value value;
		value[EASY_TAG_L_URL] = record;
		int pos = record.find_last_of('/');
		value[EASY_TAG_L_TIME] = record.substr(pos - 14, 14); // /20151123114500/*.m3u8
		root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_RECORDS].append(value);
		return true;
	}

	void EasyProtocolACK::SetHead(EasyJsonValue &header)
	{
		for (EasyJsonValue::iterator it = header.begin(); it != header.end(); ++it)
		{
			SetHeaderValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	void EasyProtocolACK::SetBody(EasyJsonValue &body)
	{
		for (EasyJsonValue::iterator it = body.begin(); it != body.end(); ++it)
		{
			SetBodyValue(it->first.c_str(), boost::apply_visitor(EasyJsonValueVisitor(), it->second));
		}
	}

	void EasyMsgSCRecordListACK::AddRecord(const string& record)
	{
		Json::Value value;
		value[EASY_TAG_L_URL] = record;
		int pos = record.find_last_of('/');
		value[EASY_TAG_L_TIME] = record.substr(pos - 14, 14); // /20151123114500/*.m3u8
		root[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_RECORDS].append(value);
	}

	//add,�Ϲ⣬start
	strDevice::strDevice() : eDeviceType(), eAppType()
	{
		snapJpgPath_.clear();
	}

	bool strDevice::GetDevInfo(const string& json)//��JSON�ı��õ��豸��Ϣ
	{
		EasyProtocol proTemp(json);
		do
		{
			string strTerminalType = proTemp.GetHeaderValue(EASY_TAG_TERMINAL_TYPE);//��ȡ�豸����
			string strAppType = proTemp.GetHeaderValue(EASY_TAG_APP_TYPE);//��ȡApp����
			serial_ = proTemp.GetBodyValue(EASY_TAG_SERIAL);//��ȡ�豸���к�

			if ((strTerminalType.size() <= 0) || (serial_.size() <= 0) || (strAppType.size() <= 0))
				break;

			eDeviceType = static_cast<EasyDarwinTerminalType>(EasyProtocol::GetTerminalType(strTerminalType));
			if (eDeviceType == -1)
				break;
			eAppType = static_cast<EasyDarwinAppType>(EasyProtocol::GetAppType(strAppType));
			if (eAppType == -1)
				break;

			name_ = proTemp.GetBodyValue(EASY_TAG_NAME);//��ȡ�豸����
			password_ = proTemp.GetBodyValue(EASY_TAG_TOKEN);//�豸��֤��
			tag_ = proTemp.GetBodyValue(EASY_TAG_TAG);//��ǩ
			channelCount_ = proTemp.GetBodyValue(EASY_TAG_CHANNEL_COUNT);//��ǰ�豸����������ͷ����

			if (eAppType == EASY_APP_TYPE_NVR)//����豸������NVR������Ҫ��ȡ����ͷ��Ϣ
			{
				//channels_.clear();//��һ�䲻��Ҫ��������߳�bug
				Json::Value *proot = proTemp.GetRoot();
				int size = (*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS].size(); //�����С 


				for (int i = 0; i < size; i++)
				{
					Json::Value &json_camera = (*proot)[EASY_TAG_ROOT][EASY_TAG_BODY][EASY_TAG_CHANNELS][i];
					EasyDevice camera;
					camera.name_ = json_camera[EASY_TAG_NAME].asString();
					camera.channel_ = json_camera[EASY_TAG_CHANNEL].asString();
					camera.status_ = json_camera[EASY_TAG_STATUS].asString();

					//channels_.push_back(camera);
					//����Ѿ����ڣ���ֻ�޸�status_���ԣ�������뵽map�С���������1���߳�д������̶߳����ü�������Ϊ������ֲ���Ԥ֪���м�ֵ��  
					//ע��NVR��������ͷ����Ϣ����״̬�ⲻӦ�÷����仯��������̲߳������ܻ��bug.
					if (channels_.find(camera.channel_) != channels_.end())//Already exist
					{
						channels_[camera.channel_].status_ = camera.status_;//change status_
					}
					else//insert
					{
						channels_[camera.channel_] = camera;
					}
				}
			}
			return true;
		} while (false);
		//ִ�е���˵���õ����豸��Ϣ�Ǵ����
		return false;
	}

	void strDevice::HoldSnapPath(const string& strJpgPath, const string& strChannel)//�������յ�ʱ������
	{
		if (EASY_APP_TYPE_CAMERA == eAppType)//���������ͷ���ͣ���ôֻ����һ��·��
		{
			snapJpgPath_ = strJpgPath;
		}
		else//�����Ҫ��������Ӧ������ͷ������
		{
			EasyDevicesIterator it;
			for (it = channels_.begin(); it != channels_.end(); ++it)
			{
				if (it->second.channel_ == strChannel)
					it->second.snapJpgPath_ = strJpgPath;
			}
		}
	}

//add,�Ϲ⣬end
}
}//namespace

