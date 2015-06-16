#pragma once

#include "commonplatform_types.h"

#define MAX_SOURCE_URL_LEN 200
#define MAX_STREAM_URL_LEN 200

#define  MAX_IDNAME_LEN 100

typedef struct stDeviceInfo
{
	uint32_t m_nId;							//设备序号, 从0开始, 如果10个设备，则从0至9

	char m_szIdname[MAX_IDNAME_LEN];		//标识名，新名字
	char m_szSourceUrl[MAX_SOURCE_URL_LEN];	//源url

}DeviceInfo;

class CParseDevice
{
public:
	CParseDevice();
	~CParseDevice();

public:
	int Init();
	void Uninit();

public:
	//device xml
	int32_t LoadDeviceXml(const char *pXmlFile);

	DeviceInfo* GetDeviceInfoByIdName(const char *pszIdName);

private:
	int32_t AddDevice(DeviceInfo& deviceInfo);
	int32_t DelDevice(uint32_t nId);
	void Clear();
};