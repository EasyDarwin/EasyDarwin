#include <map>

#include "tinyxml.h"
#include "ParseDevice.h"

using namespace std;

typedef map<uint32_t, DeviceInfo*> DeviceMap;

DeviceMap g_deviceMap;

CParseDevice::CParseDevice()
{
	g_deviceMap.clear();
}

CParseDevice::~CParseDevice()
{
	Uninit();
}

int32_t CParseDevice::Init()
{
	return success;
}

void CParseDevice::Uninit()
{
	Clear();
}

void CParseDevice::Clear()
{
	DeviceMap::iterator deviceIter = g_deviceMap.begin();
	for (; deviceIter != g_deviceMap.end(); deviceIter++)
	{
		delete (*deviceIter).second;
	}

	g_deviceMap.clear();
}

int32_t CParseDevice::AddDevice(DeviceInfo& deviceInfo)
{
	//check Id
	DeviceMap::iterator deviceIter = g_deviceMap.find(deviceInfo.m_nId);
	if (deviceIter != g_deviceMap.end())
	{
		return fail;
	}

	//new
	DeviceInfo* pDeviceInfo = new DeviceInfo;
	*pDeviceInfo = deviceInfo;

	//bool bHasUserInfo = false;
	//if ((0 != strlen(deviceInfo.m_szUser)) && (0 != strlen(deviceInfo.m_szPassword)))
	//{
	//	bHasUserInfo = true;
	//}

	//if (0 == strcmp("DH", deviceInfo.m_szIdentifier))
	//{
	//	if (deviceInfo.m_nRate)
	//	{
	//		sprintf(pDeviceInfo->m_szSourceUrl, "rtsp://%s:%d/cam/realmonitor?channel=2&subtype=1", deviceInfo.m_szIP, deviceInfo.m_nPort);
	//	} 
	//	else
	//	{
	//		sprintf(pDeviceInfo->m_szSourceUrl, "rtsp://%s:%d/cam/realmonitor?channel=2&subtype=0", deviceInfo.m_szIP, deviceInfo.m_nPort);
	//	}
	//}
	//else if (0 == strcmp("HK", deviceInfo.m_szIdentifier))
	//{
	//	if (deviceInfo.m_nRate)
	//	{
	//		sprintf(pDeviceInfo->m_szSourceUrl, "rtsp://%s:%d/h264/ch1/main/av_stream", deviceInfo.m_szIP, deviceInfo.m_nPort);
	//	} 
	//	else
	//	{
	//		sprintf(pDeviceInfo->m_szSourceUrl, "rtsp://%s:%d/h264/ch1/sub/av_stream", deviceInfo.m_szIP, deviceInfo.m_nPort);
	//	}
	//}
	//else 
	//{
	//	delete pDeviceInfo;

	//	return fail;
	//}

	//±êÊ¶Ãû
	//sprintf(pDeviceInfo->m_szIdname, "%s%s%d", deviceInfo.m_szIdentifier, deviceInfo.m_szModel, deviceInfo.m_nId);

	//insert
	g_deviceMap.insert(make_pair(pDeviceInfo->m_nId, pDeviceInfo));

	return success;
}

int32_t CParseDevice::DelDevice(uint32_t nId)
{
	//check Id
	DeviceMap::iterator deviceIter =  g_deviceMap.find(nId);
	if (deviceIter == g_deviceMap.end())
	{
		return fail;
	}

	//delete
	delete (*deviceIter).second;
	g_deviceMap.erase(deviceIter);

	return success;
}

//device xml
int32_t CParseDevice::LoadDeviceXml(const char *pXmlFile)
{
	if (NULL == pXmlFile)
	{
		return 0;
	}

	TiXmlDocument config(pXmlFile);
	if (!config.LoadFile(TIXML_ENCODING_UNKNOWN))
	{
		return 0;
	}

	//clear
	Clear();

	int32_t num_devs = 0;
	TiXmlHandle handle(&config);

	TiXmlElement* pDevElement = handle.FirstChild("Devices").FirstChild("Device").ToElement();
	while (NULL != pDevElement)
	{
		int32_t nValue = 0;
		const char* pszvalue = NULL;

		DeviceInfo devInfo;
		memset(&devInfo, 0, sizeof(devInfo));

		//id
		pszvalue = pDevElement->Attribute("id", &nValue);
		if (NULL == pszvalue) 
		{
			break;
		}
		else
		{
			devInfo.m_nId = nValue;
		}

		//streamName
		pszvalue = pDevElement->Attribute("name", &nValue);
		if (NULL == pszvalue) 
		{
			break;
		}
		else
		{
			strncpy(devInfo.m_szIdname, pszvalue, sizeof(devInfo.m_szIdname));
		}

		//url
		pszvalue = pDevElement->Attribute("url", &nValue);
		if (NULL == pszvalue) 
		{
			break;
		}
		else
		{
			strncpy(devInfo.m_szSourceUrl, pszvalue, sizeof(devInfo.m_szSourceUrl));
		}


		if (success != AddDevice(devInfo))
		{
			break;
		}

		//next element
		pDevElement = pDevElement->NextSiblingElement("Device");

		num_devs++;
	}

	return num_devs;
}

DeviceInfo* CParseDevice::GetDeviceInfoByIdName(const char *pszIdName)
{
	DeviceMap::iterator deviceIter = g_deviceMap.begin();
	for (; deviceIter != g_deviceMap.end(); deviceIter++)
	{
		DeviceInfo* pDeviceInfo = (*deviceIter).second;
		if (NULL == pDeviceInfo)
		{
			continue;
		}

		if (0 == strcmp(pszIdName, pDeviceInfo->m_szIdname))
		{
			return pDeviceInfo;
		}
	}

	return NULL;
}


DeviceInfo* CParseDevice::GetDeviceInfoByIdIndex(uint32_t nIndex)
{
	DeviceMap::iterator deviceIter = g_deviceMap.begin();
	for (; deviceIter != g_deviceMap.end(); deviceIter++)
	{
		DeviceInfo* pDeviceInfo = (*deviceIter).second;
		if (NULL == pDeviceInfo)
		{
			continue;
		}

		if (nIndex == pDeviceInfo->m_nId)
		{
			return pDeviceInfo;
		}
	}

	return NULL;
}