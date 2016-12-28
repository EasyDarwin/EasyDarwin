#include "RTSPRecordSession.h"
#include <windows.h>
#include "QTSServerInterface.h"
#include "QTSSModuleUtils.h"

// PREFS
static char*            sHttpDir = "http://127.0.0.1/Movies";
static char*            sDefaultHttpDir = "http://127.0.0.1/Movies";
static char*            sRecordDir = "./Movies";
static char*            sDefaultRecordDir = "./Movies";

void RTSPRecordSession::Initialize(QTSS_ModulePrefsObject inPrefs)
{
	sHttpDir = QTSSModuleUtils::GetStringAttribute(inPrefs, "HTTP_ROOT_DIR", sDefaultHttpDir);
	sRecordDir = QTSSModuleUtils::GetStringAttribute(inPrefs, "Record_DIR", sDefaultRecordDir);

}

DWORD WINAPI ProcessData(LPVOID lpParam)
{
	RTSPRecordSession *session = (RTSPRecordSession *)lpParam;
	while (true) {
		if (!session->readFrame()) {
			break;
		}
		Sleep(1);
	}
	return 0;
}

DWORD WINAPI ProcessStart(LPVOID lpParam)
{
	RTSPRecordSession *session = (RTSPRecordSession *)lpParam;
	
	session->rtspRead();
	
	return 0;
}

DWORD WINAPI ProcessEnd(LPVOID lpParam)
{
	RTSPRecordSession *session = (RTSPRecordSession *)lpParam;

	session->close();

	return 0;
}

RTSPRecordSession::RTSPRecordSession()
	: m_url(""),
	m_pMP4Writer(NULL),
	m_bRecording(false),
	bPlaying(false),
	hThread(nullptr)
{

}

RTSPRecordSession::~RTSPRecordSession() {
	close();
}

void RTSPRecordSession::init() {
	av_register_all();//注册组件
	avformat_network_init();//支持网络流
	m_pFormatContext = avformat_alloc_context();//初始化AVFormatContext
}

int RTSPRecordSession::rtspRead() {

	AVDictionary* options = NULL;
	av_dict_set(&options, "rtsp_transport", "tcp", 0);
	if (avformat_open_input(&m_pFormatContext,/*filepath*/sUri.c_str(), NULL, &options) != 0) {//打开文件
		printf("无法打开文件\n");
		return -1;
	}
	if (avformat_find_stream_info(m_pFormatContext, nullptr)<0)//查找流信息
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}
	m_videoIndex = -1;
	m_audioIndex = -1;
	for (int i = 0; i < m_pFormatContext->nb_streams; i++) //获取视频流ID
	{
		if (m_pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_videoIndex = i;
		}
		else if (m_pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			m_audioIndex = i;
		}
		if (m_videoIndex != -1 && m_audioIndex != -1) {
			break;
		}
	}
	if (m_videoIndex == -1)
	{
		printf("Didn't find a video stream.\n");
		return -1;
	}
	m_pCodecCtx = m_pFormatContext->streams[m_videoIndex]->codec;
	AVCodec *pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);//查找解码器
	if (pCodec == NULL)
	{
		printf("Codec not found.\n");
		return -1;
	}
	if (avcodec_open2(m_pCodecCtx, pCodec, NULL)<0)//打开解码器
	{
		printf("Could not open codec.\n");
		return -1;
	}


	int y_size = m_pCodecCtx->width * m_pCodecCtx->height;

	m_packet = (AVPacket *)malloc(sizeof(AVPacket));//存储解码前数据包AVPacket
	av_new_packet(m_packet, y_size);
	//输出一下信息-----------------------------
	printf("文件信息-----------------------------------------\n");
	av_dump_format(m_pFormatContext, 0, "./log.txt", 0);
	printf("-------------------------------------------------\n");

	bPlaying = true;

	hThread = CreateThread(NULL, 0, ProcessData, this, 0, NULL);
}
int RTSPRecordSession::play(const string& url,const string& subName) {

	fSubName = subName.substr(0,subName.size() - 6);
	sUri = url.substr(0, url.size() - 2);
	
	hThread = CreateThread(NULL, 0, ProcessStart, this, 0, NULL);
	
	return 0;
	
}

bool RTSPRecordSession::readFrame() {
	OSMutex *mutex = &fMutex;
	if (!mutex->TryLock()) {
		return true;
	}
	//locker.Lock();
	//printf("写入一帧");
	if (!bPlaying || !m_pFormatContext) {
		mutex->Unlock();
		return false;
	}
	int got_picture = 0;
	static struct SwsContext *img_convert_ctx;
	bool result = false;
	if (av_read_frame(m_pFormatContext, m_packet) < 0)//循环获取压缩数据包AVPacket
	{
		mutex->Unlock();
		return true;
	}
	
	if (m_packet->stream_index == m_videoIndex)
	{
		bool bKeyFrame = m_packet->flags | AV_PKT_FLAG_KEY;
		;
		if (bKeyFrame)
		{
			if (!m_bRecording)
			{
				CreateNewMp4Writer(44100, 2);
			}
			else {
				SInt64 curTime = OS::Milliseconds();
				if (curTime - fCreateTime >= 5 * 60 * 1000 && bPlaying) {
					CloseMP4Writer();
					CreateNewMp4Writer(44100, 2);
				}
			}
		}
		if (m_bRecording)
		{
			WriteMP4VideoFrame((unsigned char*)m_packet->data, m_packet->size, bKeyFrame, clock(), m_pCodecCtx->width, m_pCodecCtx->height);
		}

	}
	if (m_packet->stream_index == m_audioIndex)
	{
		if (m_bRecording)
		{
			WriteMP4AudioFrame((unsigned char*)m_packet->data, m_packet->size, clock());
		}
	}
	av_free_packet(m_packet);
	mutex->Unlock();
	//av_free(m_pFrameYUV);
	return true;
}

void RTSPRecordSession::close() {

	if (bPlaying) {
		bPlaying = false;
		
		OSMutex *mutex = &fMutex;
		mutex->Lock();
		{
			CloseMP4Writer();
			printf("结束写入");
			//locker.Lock();
			CloseHandle(hThread);
			hThread = nullptr;

			avcodec_close(m_pCodecCtx);
			avformat_close_input(&m_pFormatContext);
			mutex->Unlock();
		}
		//locker.Unlock();
	}
}

void RTSPRecordSession::stop(){

	hThread = CreateThread(NULL, 0, ProcessEnd, this, 0, NULL);
	

}
	

int RTSPRecordSession::CreateNewMp4Writer(int sample_rate,int channels) {

	
	if (!bPlaying) {

		return false;
	}

	char *movieFolder = sRecordDir;

	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char fPlayName[QTSS_MAX_NAME_LENGTH] = { 0 };
	char subDir[QTSS_MAX_NAME_LENGTH] = { 0 };
	qtss_sprintf(subDir, "%s/", fSubName.c_str());

	//char rootDir[QTSS_MAX_FILE_NAME_LENGTH] = { 0 };
	//qtss_sprintf(rootDir,"%s/", movieFolder);

	memset(fPlayName, 0, QTSS_MAX_NAME_LENGTH);

	//qtss_sprintf(rootDir,"%s/%s/%4d%02d%02d_%02d%02d%02d.mp4", movieFolder,subDir,sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute, sys.wSecond);

	qtss_sprintf(fPlayName, "%s/MP4/", movieFolder);
	mkdir(fPlayName);
	qtss_sprintf(fPlayName, "%s/MP4/%s/", movieFolder, subDir);
	mkdir(fPlayName);
	qtss_sprintf(fPlayName, "%s/MP4/%s/%4d%02d%02d_%02d%02d%02d.mp4", movieFolder, subDir, sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);

	//fileName("./xmgj_%4d%02d%02d_%02d%02d%02d.mp4",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute, sys.wSecond);
	fCreateTime = OS::Milliseconds();
	//m_currInfo->startTime = fCreateTime;
	CreateMP4Writer(fPlayName, sample_rate, channels, 16, ZOUTFILE_FLAG_FULL);
	return 0;
}

//写MP4文件(录制相关)
int RTSPRecordSession::CreateMP4Writer(char* sFileName,  int nSampleRate, int nChannel, int nBitsPerSample, int nFlag)
{
	if (m_bRecording)
	{
		return -1;
	}

	{
		if (!m_pMP4Writer)
		{
			m_pMP4Writer = new EasyMP4Writer();
		}
		if (!m_pMP4Writer->CreateMP4File(sFileName, nFlag))
		{
			delete m_pMP4Writer;
			m_pMP4Writer = NULL;
			return -1;
		}

		// 		前五个字节为 AAC object types  LOW          2
		// 		接着4个字节为 码率index        16000        8
		// 		采样标志标准：
		//	static unsigned long tnsSupportedSamplingRates[13] = //音频采样率标准（标志），下标为写入标志
		//	{ 96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,0 };

		// 		接着4个字节为 channels 个数                 2
		// 		应打印出的正确2进制形式为  00010 | 1000 | 0010 | 000
		// 														2        8         2
		//  BYTE ubDecInfoBuff[] =  {0x12,0x10};//00010 0100 0010 000

		//音频采样率标准（标志），下标为写入标志
		unsigned long tnsSupportedSamplingRates[13] = { 96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,0 };
		int nI = 0;
		for (nI = 0; nI<13; nI++)
		{
			if (tnsSupportedSamplingRates[nI] == nSampleRate)
			{
				break;
			}
		}
		unsigned char ucDecInfoBuff[2] = { 0x12,0x10 };//

		unsigned short  nDecInfo = (1 << 12) | (nI << 7) | (nChannel << 3);
		int nSize = sizeof(unsigned short);
		memcpy(ucDecInfoBuff, &nDecInfo, nSize);
		SWAP(ucDecInfoBuff[0], ucDecInfoBuff[1]);
		int unBuffSize = sizeof(ucDecInfoBuff) * sizeof(unsigned char);

		m_pMP4Writer->WriteAACInfo(ucDecInfoBuff, unBuffSize, nSampleRate, nChannel, nBitsPerSample);
	}
	m_bRecording = 1;

	return 1;
}

int RTSPRecordSession::WriteMP4VideoFrame(unsigned char* pdata, int datasize, bool keyframe, long nTimestamp, int nWidth, int nHeight)
{

	{
		if (m_pMP4Writer)
		{
			m_pMP4Writer->WriteMp4File(pdata, datasize, keyframe, nTimestamp, nWidth, nHeight);
		}
	}

	return 1;
}

int RTSPRecordSession::WriteMP4AudioFrame(unsigned char* pdata, int datasize, long timestamp)
{

	{
		if (m_pMP4Writer)
		{
			if (m_pMP4Writer->CanWrite())
			{
				return m_pMP4Writer->WriteAACFrame(pdata, datasize, timestamp);
			}
		}
	}

	return 0;
}
void RTSPRecordSession::CloseMP4Writer()
{
	//fclose(h264);
	m_bRecording = 0;

	{
		if (m_pMP4Writer)
		{
			//WriteConfig();
			m_pMP4Writer->SaveFile();
			delete m_pMP4Writer;
			m_pMP4Writer = NULL;
		}
	}

}

char *RTSPRecordSession::getRecordRootPath() {
	return sRecordDir;
}

char *RTSPRecordSession::getNetRecordRootPath() {
	return sHttpDir;
}