#pragma once

#include "commonplatform_types.h"

#define MAX_SOURCE_URL_LEN 200
#define MAX_STREAM_URL_LEN 200

#define  MAX_TYPE_LEN 10
#define  MAX_IP_LEN 16
#define  MAX_USERNAME_LEN 30
#define  MAX_PASSWORD_LEN 30
#define  MAX_IDNAME_LEN 100
#define  MAX_MODEL_LEN 50

typedef struct stDeviceInfo
{
	uint32_t m_nId;							//设备序号, 从0开始, 如果10个设备，则从0至9
	char m_szIdentifier[MAX_TYPE_LEN];		//厂家标识, DH为大华, HK为海康

	char m_szModel[MAX_MODEL_LEN];			//型号

	char m_szIP[MAX_IP_LEN];				//设备IP地址
	uint16_t m_nPort;						//设备端口

	char m_szUser[MAX_USERNAME_LEN];		//用户名
	char m_szPassword[MAX_PASSWORD_LEN];	//密码

	uint8_t m_nRate;						//码流类型, 0为主码流, 1为子码流

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