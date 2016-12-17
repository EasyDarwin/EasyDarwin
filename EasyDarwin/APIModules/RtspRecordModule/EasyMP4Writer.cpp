/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
// Add by SwordTwelve
#include "EasyMP4Writer.h"

unsigned char* EasyMP4Writer::FindNal(unsigned char*buff,int inlen,int&outlen,bool& end)
{
	unsigned char*tempstart=NULL;
	unsigned char*search=buff+2;
	unsigned char*searchper1=buff;
	unsigned char*searchper2=buff+1;
	outlen=0;
	end=false;
	while((search-buff)<inlen)
	{
		if (search[0]==0x01&&searchper1[0]==0x00&&searchper2[0]==0x00)
		{
			if (tempstart==NULL)
			{
				tempstart=search+1;
			}
			else
			{
				outlen=search-tempstart-3+1;
				break;
			}
		}
		searchper2=searchper1;
		searchper1=search;
		search++;
	}
	if (outlen==0&&tempstart!=NULL)
	{
		outlen=search-tempstart;
		end=true;
	}
	if (tempstart==NULL)
	{
		end=true;
	}
	return tempstart;
}

//STD Find Nal unit
/** this function is taken from the h264bitstream library written by Alex Izvorski and Alex Giladi
 Find the beginning and end of a NAL (Network Abstraction Layer) unit in a byte buffer containing H264 bitstream data.
 @param[in]   buf        the buffer
 @param[in]   size       the size of the buffer
 @param[out]  nal_start  the beginning offset of the nal
 @param[out]  nal_end    the end offset of the nal
 @return                 the length of the nal, or 0 if did not find start of nal, or -1 if did not find end of nal
 */
int EasyMP4Writer::find_nal_unit(unsigned char* buf, int size, int* nal_start, int* nal_end)
{
    int i;
    // find start
    *nal_start = 0;
    *nal_end = 0;

    i = 0;
    while (   //( next_bits( 24 ) != 0x000001 && next_bits( 32 ) != 0x00000001 )
           (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01) &&
           (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0 || buf[i+3] != 0x01)
           )
    {
        i++; // skip leading zero
        if (i+4 >= size) { return 0; } // did not find nal start
    }

    if  (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01) // ( next_bits( 24 ) != 0x000001 )
    {
        i++;
    }

    if  (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01) { /* error, should never happen */ return 0; }
    i+= 3;
    *nal_start = i;

    while (   //( next_bits( 24 ) != 0x000000 && next_bits( 24 ) != 0x000001 )
           (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0) &&
           (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01)
           )
    {
        i++;
        // FIXME the next line fails when reading a nal that ends exactly at the end of the data
        if (i+3 >= size) { *nal_end = size; return -1; } // did not find nal end, stream ended first
    }

    *nal_end = i;
    return (*nal_end - *nal_start);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
EasyMP4Writer::EasyMP4Writer()
{
	m_videtrackid=-1;
	m_audiotrackid=-1;
	m_audiostartimestamp=-1;
	m_videostartimestamp=-1;

	m_flag=0;
	p_file=NULL;
	p_config=NULL;
	p_videosample=NULL;
	p_audiosample=NULL;

	m_psps=NULL;
	m_ppps=NULL;
	m_spslen=0;
	m_ppslen=0;
	m_bwritevideoinfo = false;
}

EasyMP4Writer::~EasyMP4Writer()
{
	SaveFile();
}
bool EasyMP4Writer::CreateMP4File(char*filename,int flag)
{
	SaveFile();
	m_audiostartimestamp=-1;
	m_videostartimestamp=-1;
	if(filename==NULL)
	{
		char filename2[256]={0};
		sprintf_s(filename2,"%d-gpac%d.mp4",time(NULL),rand());
		p_file=gf_isom_open(filename2,GF_ISOM_OPEN_WRITE,NULL);//打开文件
	}else
		p_file=gf_isom_open(filename,GF_ISOM_OPEN_WRITE,NULL);//打开文件

	if (p_file==NULL)
	{
		return false;
	}

	gf_isom_set_brand_info(p_file,GF_ISOM_BRAND_MP42,0);

	if(flag&ZOUTFILE_FLAG_VIDEO)
	{
		m_videtrackid=gf_isom_new_track(p_file,0,GF_ISOM_MEDIA_VISUAL,1000);
		gf_isom_set_track_enabled(p_file,m_videtrackid,1);
	}
	if(flag&ZOUTFILE_FLAG_AUDIO)
	{
		m_audiotrackid=gf_isom_new_track(p_file,0,GF_ISOM_MEDIA_AUDIO,1000);
		gf_isom_set_track_enabled(p_file,m_audiotrackid,1);
	}
	return true;
}
//sps,pps第一个字节为0x67或68,
bool EasyMP4Writer::WriteH264SPSandPPS(unsigned char*sps,int spslen,unsigned char*pps,int ppslen,int width,int height)
{	

	p_videosample=gf_isom_sample_new();
	p_videosample->data=(char*)malloc(1024*1024);


	p_config=gf_odf_avc_cfg_new();	
	gf_isom_avc_config_new(p_file,m_videtrackid,p_config,NULL,NULL,&i_videodescidx);
	gf_isom_set_visual_info(p_file,m_videtrackid,i_videodescidx,width,height);

	GF_AVCConfigSlot m_slotsps={0};
	GF_AVCConfigSlot m_slotpps={0};
	
	p_config->configurationVersion = 1;
	p_config->AVCProfileIndication = sps[1];
	p_config->profile_compatibility = sps[2];
	p_config->AVCLevelIndication = sps[3];
	
	m_slotsps.size=spslen;
	m_slotsps.data=(char*)malloc(spslen);
	memcpy(m_slotsps.data,sps,spslen);	
	gf_list_add(p_config->sequenceParameterSets,&m_slotsps);
	
	m_slotpps.size=ppslen;
	m_slotpps.data=(char*)malloc(ppslen);
	memcpy(m_slotpps.data,pps,ppslen);
	gf_list_add(p_config->pictureParameterSets,&m_slotpps);
	
	gf_isom_avc_config_update(p_file,m_videtrackid,1,p_config);

	free(m_slotsps.data);
	free(m_slotpps.data);

	return true;
}
//写入AAC信息
bool EasyMP4Writer::WriteAACInfo(unsigned char*info,int len, int nSampleRate, int nChannel, int nBitsPerSample)
{
	p_audiosample=gf_isom_sample_new();
	p_audiosample->data=(char*)malloc(1024*10);

	GF_ESD*esd=	gf_odf_desc_esd_new(0);
	esd->ESID=gf_isom_get_track_id(p_file,m_audiotrackid);
	esd->OCRESID=gf_isom_get_track_id(p_file,m_audiotrackid);
	esd->decoderConfig->streamType=0x05;
	esd->decoderConfig->objectTypeIndication=0x40;//0x40;
	esd->slConfig->timestampResolution=1000;//1000;//时间单元	
	esd->decoderConfig->decoderSpecificInfo=(GF_DefaultDescriptor*)gf_odf_desc_new(GF_ODF_DSI_TAG);

	esd->decoderConfig->decoderSpecificInfo->data=(char*)malloc(len);
	memcpy(esd->decoderConfig->decoderSpecificInfo->data,info,len);
	esd->decoderConfig->decoderSpecificInfo->dataLength=len;
	

	GF_Err gferr=gf_isom_new_mpeg4_description(p_file, m_audiotrackid, esd,  NULL, NULL, &i_audiodescidx);
	if (gferr!=0)
	{
//		TRACE("mpeg4_description:%d\n",gferr);
	}
	gferr=gf_isom_set_audio_info(p_file,m_audiotrackid,i_audiodescidx, nSampleRate,nChannel, nBitsPerSample);//44100 2 16
	if (gferr!=0)
	{
//		TRACE("gf_isom_set_audio:%d\n",gferr);
	}

	free(esd->decoderConfig->decoderSpecificInfo->data);


	return true;
}
//写入一帧，前四字节为该帧NAL长度
bool EasyMP4Writer::WriteH264Frame(unsigned char*data,int len,bool keyframe,long timestamp)
{		
	if (m_videostartimestamp==-1&&keyframe)
	{
		m_videostartimestamp=timestamp;
	}
	if (m_videostartimestamp!=-1)
	{
		p_videosample->IsRAP=keyframe;
		p_videosample->dataLength=len;
		memcpy(p_videosample->data,data,len);
		p_videosample->DTS=timestamp-m_videostartimestamp;
		p_videosample->CTS_Offset=0;	
		GF_Err gferr=gf_isom_add_sample(p_file,m_videtrackid,i_videodescidx,p_videosample);			
		if (gferr==-1)
		{
			p_videosample->DTS=timestamp-m_videostartimestamp+15;
			gf_isom_add_sample(p_file,m_videtrackid,i_videodescidx,p_videosample);
		}
	}
	
	return true;
}
//写入aac数据，只有raw数据
bool EasyMP4Writer::WriteAACFrame(unsigned char*data,int len,long timestamp)
{
		if (m_audiostartimestamp==-1)
		{
			m_audiostartimestamp=timestamp;
		}
		p_audiosample->IsRAP=1;
		p_audiosample->dataLength=len;
		memcpy(	p_audiosample->data,data,len);
		p_audiosample->DTS=timestamp-m_audiostartimestamp;
		p_audiosample->CTS_Offset=0;	
		GF_Err gferr=gf_isom_add_sample(p_file,m_audiotrackid,i_audiodescidx,p_audiosample);			
		if (gferr==-1)
		{
			p_audiosample->DTS=timestamp-m_audiostartimestamp+15;
			gf_isom_add_sample(p_file,m_audiotrackid,i_audiodescidx,p_audiosample);
		}
	return true;
}
//保存文件
bool EasyMP4Writer::SaveFile()
{
	if (m_psps)
	{
		delete m_psps;
		m_psps = NULL;
	}
	if (m_ppps)
	{
		delete m_ppps;
		m_ppps = NULL;
	}
	m_spslen=0;
	m_ppslen=0;

	m_audiostartimestamp=-1;
	m_videostartimestamp=-1;
	if (p_file)
	{
		gf_isom_close(p_file);
		p_file=NULL;
	}
	if(p_config)
	{
	//	delete p_config->pictureParameterSets;
		p_config->pictureParameterSets=NULL;
	//	delete p_config->sequenceParameterSets;
		p_config->sequenceParameterSets=NULL;
		gf_odf_avc_cfg_del(p_config);
		p_config=NULL;
	}
	if(	p_audiosample)
	{
		if(	p_audiosample->data)
		{
			free(p_audiosample->data);
			p_audiosample->data=NULL;
		}
		gf_isom_sample_del(&p_audiosample);
		p_audiosample=NULL;
	}

	if(	p_videosample)
	{
		if(	p_videosample->data)
		{
			free(p_videosample->data);
			p_videosample->data=NULL;
		}
		gf_isom_sample_del(&p_videosample);
		p_audiosample=NULL;
	}
	return true;
}

bool EasyMP4Writer::CanWrite()
{
	return m_bwritevideoinfo;
}

// int EasyMP4Writer::WriteMp4File(BYTE* pdata, int datasize, bool keyframe, long nTimestamp, int nWidth, int nHeight)
// {
// 	if (nTimestamp==0||(pdata==NULL)||datasize<=0)
// 	{
// 		return -1;
// 	}
// 	int inlen=datasize;
// 	unsigned char*pin=pdata;
// 	int outlen=0;
// 	unsigned char*pout=NULL;
// 	bool bend = false;
// 
// 	int datalen=0;
// 	BOOL bSPSOrPPS = FALSE;
// 
// 	int iOutLen = datasize;
// 
// 	BYTE* pRealData = new BYTE[datasize+4];
// 	int nRealDataSize = 0;
// 	memset(pRealData,0x00, datasize+4);
// 	do 
// 	{
// 		//pout=FindNal(pin,inlen,outlen,bend);
// 		int nal_start = 0;
// 		int nal_end = 0;
// 		 outlen = find_nal_unit(pin,inlen, &nal_start, &nal_end );
// 		if (outlen<=0)
// 		{
// 			break;
// 		}
// 		pout = pin+nal_start;
// 		if(pout!=NULL)
// 		{
// 			//	TRACE("%x len:%d ",pout[0],outlen);
// 			if (pout[0]==0x27||pout[0]==0x67)//0X67
// 			{
// // 				m_psps=pout;
// // 				m_spslen=outlen;
// 				//pout[0] = 0x67;
// 				if(m_bwritevideoinfo==false)
// 				{
// 					m_psps = new unsigned char[outlen];
// 					memcpy(m_psps, pout, outlen);
// 					m_spslen=outlen;
// 				}
// 				bSPSOrPPS = TRUE;
// 			}
// 			else if (pout[0]==0x28||pout[0]==0x68)//0X68
// 			{
// 				// 				m_ppps=pout;
// 				// 				m_ppslen=outlen;
// 				//pout[0] = 0x68;
// 				if(m_bwritevideoinfo==false)
// 				{
// 					m_ppps = new unsigned char[outlen];//outlen
// 					memcpy(m_ppps, pout, outlen);
// 					m_ppslen = outlen;
// 				}
// 				bSPSOrPPS = TRUE;
// 			}
// // 			else if (pout[0] == 0x06)//SEI
// // 			{
// // 
// // 			}
// 			else
// 			{
// 				nRealDataSize = datasize- (pout-pdata) ;
// 				memcpy(pRealData+4, pout, nRealDataSize);
// // 				memcpy(pRealData+nRealDataSize+4, pout, outlen);
// // 				nRealDataSize += outlen;
// 					break;
// 			}
// 
// // 			if (pout[0] == 0x65 && pout[1] == 0x88)
// // 			{
// // 				pout = pout;
// // 				outlen = datasize- (pout-pdata);
// // 				break;
// // 			}
// // 			if (pout[0] == 0x41 && pout[1] == 0x9A)
// // 			{
// // 				pout = pdata+4;
// // 				outlen = datasize-4;
// // 				break;
// // 			}
// 			inlen=inlen-outlen-(pout-pin);
// 			pin=pout+outlen;
// 		}
// 	} while (bend!=true);
// 
// 	if (m_bwritevideoinfo==FALSE&&m_ppps&&m_psps)
// 	{
// 		// PPS末尾的0过滤,否则VLC可能播放不出来 [12/22/2015 Dingshuai]
// 		int nPPSSize = m_ppslen;
// 		int nZeroCount = 0;
// 		for (int nI = nPPSSize-1; nI>=0; nI--)
// 		{
// 			if (m_ppps[nI] == 0x00)
// 			{
// 				nZeroCount++;
// 			}
// 			else
// 			{
// 				break;
// 			}
// 		}
// 		m_ppslen = m_ppslen-nZeroCount;
// 		WriteH264SPSandPPS(m_psps,m_spslen,m_ppps,m_ppslen,nWidth,nHeight);
// 		m_bwritevideoinfo = TRUE;
// 	}
// 	if (m_bwritevideoinfo==FALSE||nRealDataSize<=0 )
// 	{
// 		return 0;//获取sps pps失败
// 	}
// 
// 	if(/*bSPSOrPPS*/pout[0]==0x67 || pout[0]==0x68)
// 	{
// 		return 0;
// 	}
// 
// 
// 	memcpy(pRealData, &nRealDataSize, 4);
// 
// 	//写入头4个字节==nal内容的长度(H264数据的长度)
// 	BYTE byte0 = pRealData[3];
// 	BYTE byte1 = pRealData[2];
// 	pRealData[3] = pRealData[0];
// 	pRealData[2] = pRealData[1];
// 	pRealData[1] = byte1;
// 	pRealData[0] = byte0;
// 	//memcpy(pRealData+4, pout, outlen);
// 
// 	WriteH264Frame(pRealData, nRealDataSize+4, keyframe, nTimestamp);//左移4单位，加上数据长度头？
// 
// 	if (pRealData)
// 	{	
// 		delete []pRealData;
// 		pRealData = NULL;
// 	}
// 
// 	return TRUE;
// }

// 只支持一帧单Nal的x264编码的H264帧数据 [12/30/2015 Dingshuai]
int EasyMP4Writer::WriteMp4File(unsigned char* pdata, int datasize, bool keyframe, long nTimestamp, int nWidth, int nHeight)
{
	if (nTimestamp==0||(pdata==NULL)||datasize<=0)
	{
		return -1;
	}
	int inlen=datasize;
	unsigned char*pin=pdata;
	int outlen=0;
	unsigned char*pout=NULL;
	bool bend = false;

	int datalen=0;
	bool bSPSOrPPS = false;

	do 
	{
		pout=FindNal(pin,inlen,outlen,bend);
		if(pout!=NULL)
		{
			//	TRACE("%x len:%d ",pout[0],outlen);
			if (pout[0]==0x27||pout[0]==0x67)//0X67
			{
// 				m_psps=pout;
// 				m_spslen=outlen;
				//pout[0] = 0x67;
				if(m_bwritevideoinfo==false)
				{
					m_psps = new unsigned char[outlen];
					memcpy(m_psps, pout, outlen);
					m_spslen=outlen;
				}
				bSPSOrPPS = true;
			}
			if (pout[0]==0x28||pout[0]==0x68)//0X68
			{
				// 				m_ppps=pout;
				// 				m_ppslen=outlen;
				//pout[0] = 0x68;
				if(m_bwritevideoinfo==false)
				{
					m_ppps = new unsigned char[outlen];
					memcpy(m_ppps, pout, outlen);
					m_ppslen = outlen;
				}
				bSPSOrPPS = true;
			}
			inlen=inlen-outlen-(pout-pin);
			pin=pout+outlen;
		}
	} while (bend!=true);

	if (m_bwritevideoinfo==false&&m_ppps&&m_psps)
	{
		// PPS末尾的0过滤,否则VLC可能播放不出来 [12/22/2015 Dingshuai]
		int nPPSSize = m_ppslen;
		int nZeroCount = 0;
		for (int nI = nPPSSize-1; nI>=0; nI--)
		{
			if (m_ppps[nI] == 0x00)
			{
				nZeroCount++;
			}
			else
			{
				break;
			}
		}
		m_ppslen = m_ppslen-nZeroCount;
		WriteH264SPSandPPS(m_psps,m_spslen,m_ppps,m_ppslen,nWidth,nHeight);
		m_bwritevideoinfo = true;
	}
	if (m_bwritevideoinfo==false||pout==NULL )
	{
		return 0;//获取sps pps失败
	}

	if(/*bSPSOrPPS*/pout[0]==0x67 || pout[0]==0x68)
	{
		return 0;
	}

	int iOutLen = outlen;
	unsigned char* pRealData = new unsigned char[outlen+4];
	memset(pRealData,0x00, outlen+4);
	memcpy(pRealData, &iOutLen, 4);

	//写入头4个字节==nal内容的长度(H264数据的长度)
	unsigned char byte0 = pRealData[3];
	unsigned char byte1 = pRealData[2];
	pRealData[3] = pRealData[0];
	pRealData[2] = pRealData[1];
	pRealData[1] = byte1;
	pRealData[0] = byte0;
	memcpy(pRealData+4, pout, outlen);

	WriteH264Frame(pRealData,outlen+4, keyframe, nTimestamp);//左移4单位，加上数据长度头？

	if (pRealData)
	{	
		delete []pRealData;
		pRealData = NULL;
	}

	return true;
}