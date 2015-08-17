/*
    Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
    Github: https://github.com/EasyDarwin
    WEChat: EasyDarwin
    Website: http://www.easydarwin.org
 */
/* 
 *	File:   main.cpp
 *	Author: wellsen@easydariwn.org
 *	
 *	Created on March 28, 2015, 12:01 AM
 */

#include <stdio.h>
#include "revision.h"
#include "EasyCMSAPI.h"
#include <string.h>
#include <stdlib.h>
#include "inifiles.h"
#ifndef _WIN32
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>
#else
#include <Windows.h>
#endif

static void initialise(void)
{
#ifdef _WIN32
    WSADATA data;
    if (WSAStartup(MAKEWORD(1, 1), &data) != 0)
    {
        fputs("Could not initialise Winsock.\n", stderr);
        exit(1);
    }
#endif
}

static void uninitialise(void)
{
#ifdef _WIN32
    WSACleanup();
#endif
}

using namespace std;
using namespace EasyDarwin::libEasyCMS;


static int exit_flag = 0;

static void TerminateHandler(int sigNumber)
{
    printf("EasyCamera terminated[%d]\n", sigNumber);
    exit_flag = sigNumber;
    exit(0);
}

void myEventCallback(EASYDARWIN_EVENT_TYPE eEvent, const char* pEventData, unsigned int iDataLen, void* pUserData)
{
	switch(eEvent)
	{
	case EASYDARWIN_EVENT_LOGIN:
		printf("Device On Line!\n");
		break;
	case EASYDARWIN_EVENT_OFFLINE:
		printf("Device Offline! \n");
		break;
	default:
		break;
	}
}

void LogCallback(const char* msg, void* pClient)
{
    printf("\n%s\n", msg);
}

int run()
{
	//设备SN
    string sDeviceSerial = IniReadString("server", "serial", "", "easycamera.ini");
	//设备密码
    string sDevicePassword = IniReadString("server", "password", "admin", "easycamera.ini");
	//CMS地址
    string sCMSAddr = IniReadString("server", "cms_addr", "www.easydarwin.org", "easycamera.ini");
	//CMS端口
    int iCMSPort = IniReadInteger("server", "cms_port", 10000, "easycamera.ini");
    
	printf("device[%s]/v%s (Build/%s) is now running\n", sDeviceSerial.c_str(), kVersionString, kBuildString);
    
	EasyDarwinCMSAPI api;

	api.SetEventCallBack(myEventCallback, NULL);
    
    Easy_Error theErr = Easy_NoErr;
    do
    {
		theErr = api.Login(sCMSAddr.c_str(), iCMSPort, sDeviceSerial.c_str(), sDevicePassword.c_str());
        if (Easy_NoErr != Easy_NoErr)
        {
            //如果向消息中心发送Login命令失败,相当于整个设备端启动失败
#ifndef _WIN32
            sleep(10);
#else
            Sleep(100);
#endif

            //需要实现自检机制

            //定期重复向消息中心投递Login命令
            continue;

            //返回Easy_NoErr表示投递成功,后面就全部由消息中心接管

        }
        else
        {
            //登录成功，交给libEasyCMS接管
            break;
        }
    }
    while (true);


    while (true)
    {
#ifndef _WIN32
        sleep(1);
#else
        Sleep(1000);
#endif
    }

    return 0;
}

#ifndef _WIN32

#define CRASH_LOG "easycamera.crash"

string mktimestr()
{
    struct tm timeval;
    time_t timecal;
    char str[64];
    if(time(&timecal) != -1)
    {
        localtime_r(&timecal, &timeval);
        sprintf(str, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
                timeval.tm_year + 1900,
                timeval.tm_mon + 1,
                timeval.tm_mday,
                timeval.tm_hour, timeval.tm_min, timeval.tm_sec);
        return string(str);
    }
    return string();
}

void write_log(const char *file, const char *msg)
{
    FILE *fp = fopen(file, "a+");
    
    if(fp != NULL)
    {
        fprintf(fp, "%s %s\n", mktimestr().c_str(), msg);
        fclose(fp);
    }
}

pid_t Start()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        if(run() < 0)
        {
            printf("device run failed\n");
            exit(EXIT_FAILURE);
            return -1;
        }
    }
    char msg[128];
    sprintf(msg, "create device child process:%d", pid);
    write_log(CRASH_LOG, msg);
    return pid;
}
#endif


/*
 * 
 */
int main(int argc, char** argv)
{
#if 0
    signal(SIGTERM, TerminateHandler);
    signal(SIGINT, TerminateHandler);
    signal(SIGKILL, TerminateHandler);
    signal(SIGQUIT, TerminateHandler);
    signal(SIGSTOP, TerminateHandler);
    signal(SIGTSTP, TerminateHandler);

    signal(SIGABRT, TerminateHandler);
    signal(SIGBUS, TerminateHandler);
    signal(SIGFPE, TerminateHandler);
    signal(SIGILL, TerminateHandler);
    signal(SIGIOT, TerminateHandler);
    signal(SIGSEGV, TerminateHandler);
    signal(SIGSYS, TerminateHandler);
    signal(SIGTRAP, TerminateHandler);
    signal(SIGXCPU, TerminateHandler);
    signal(SIGXFSZ, TerminateHandler);

    signal(SIGUSR1, TerminateHandler);
    signal(SIGUSR2, TerminateHandler);
#endif
    initialise();

#ifdef _WIN32
    run();
#else
    //run();
#if 1   
    pid_t pid = Start();
    
    char msg[512];

    while (true) 
    {
       int status;
        pid_t id = waitpid(pid, &status, WNOHANG);        
        if(id == -1)
        {          
            sprintf(msg, "wait device pid[%d] failed：%s\n", pid, strerror(errno)); 
            write_log(CRASH_LOG, msg);
        }
        else if(id == 0)
        {   
            sleep(2);
            continue;
        }
        else if(id == pid)
        {        
            if(WIFEXITED(status))
            {
                sprintf(msg, "device pid[%d] exit code:[%d]", id, WEXITSTATUS(status));
                write_log(CRASH_LOG, msg);
            }
            else if (WIFSIGNALED(status))
            {
                sprintf(msg, "device pid[%d] exit due to signal[%d]", id, WTERMSIG(status));
                write_log(CRASH_LOG, msg);
            }
            else
            {
                sprintf(msg, "device pid[%d] dying", id);
                write_log(CRASH_LOG, msg);
            }
            pid = Start();
        }
        sleep(2);
    }
#endif
#endif 

    uninitialise();
    return 0;
}

