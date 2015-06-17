/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
    File:       QTSServer.h

    Contains:   This object is responsible for bringing up & shutting down
                the server. It also loads & initializes all modules.
*/

#ifndef __QTSSERVER_H__
#define __QTSSERVER_H__

#include "QTSServerInterface.h"
#include "Task.h"

class ServiceListenerSocket;
class RedisSession;

class QTSServer : public QTSServerInterface
{
    public:

        QTSServer() {}
        virtual ~QTSServer();

        //
        // Initialize
        //
        // This function starts the server. If it returns true, the server has
        // started up sucessfully. If it returns false, a fatal error occurred
        // while attempting to start the server.
        //
        // This function *must* be called before the server creates any threads,
        // because one of its actions is to change the server to the right UID / GID.
        // Threads will only inherit these if they are created afterwards.
        Bool16 Initialize(XMLPrefsParser* inPrefsSource, PrefsSource* inMessagesSource,
                            UInt16 inPortOverride, Bool16 createListeners);
        
        //
        // InitModules
        //
        // Initialize *does not* do much of the module initialization tasks. This
        // function may be called after the server has created threads, but the
        // server must not be in a state where it can do real work. In other words,
        // call this function right after calling Initialize.                   
        void InitModules(QTSS_ServerState inEndState);
        
        //
        // StartTasks
        //
        // The server has certain global tasks that it runs for things like stats
        // updating and RTCP processing. This function must be called to start those
        // going, and it must be called after Initialize                
        void StartTasks();

        //
        // RereadPrefsService
        //
        // This service is registered by the server (calling "RereadPreferences").
        // It rereads the preferences. Anyone can call this to reread the preferences,
        // and it may be called safely at any time, though it will fail with a
        // QTSS_OutOfState if the server isn't in the qtssRunningState.
        
        static QTSS_Error RereadPrefsService(QTSS_ServiceFunctionArgsPtr inArgs);

        //
        // CreateListeners
        //
        // This function may be called multiple times & at any time.
        // It updates the server's listeners to reflect what the preferences say.
        // Returns false if server couldn't listen on one or more of the ports, true otherwise
        Bool16                  CreateListeners(Bool16 startListeningNow, QTSServerPrefs* inPrefs, UInt16 inPortOverride);

        //
        // SetDefaultIPAddr
        //
        // Sets the IP address related attributes of the server.
        Bool16                  SetDefaultIPAddr();
                
        Bool16                  SwitchPersonality();

     private:
        static XMLPrefsParser* sPrefsSource;
        static PrefsSource* sMessagesSource;

		RedisSession*			fRedisSession;
        
        //
        // Module loading & unloading routines
        
        static QTSS_Callbacks   sCallbacks;
        
        // Sets up QTSS API callback routines
        void                    InitCallbacks();
        
        // Loads compiled-in modules
        void                    LoadCompiledInModules();

        // Loads modules from disk
        void                    LoadModules(QTSServerPrefs* inPrefs);
        void                    CreateModule(char* inModuleFolderPath, char* inModuleName);
        
        // Adds a module to the module array
        Bool16                  AddModule(QTSSModule* inModule);
        
        // Call module init roles
        void                    DoInitRole();
        UInt32*                 GetBindIPAddrs(QTSServerPrefs* inPrefs, UInt32* outNumAddrsPtr);
        UInt16					GetServicePorts(QTSServerPrefs* inPrefs);
        
        // Build & destroy the optimized role / module arrays for invoking modules
        void                    BuildModuleRoleArrays();
        void                    DestroyModuleRoleArrays();

#ifndef __Win32__
        static pid_t            sMainPid;
#endif

};

class RereadPrefsTask : public Task
{
public:
    virtual SInt64 Run()
    {
        QTSServer::RereadPrefsService(NULL);
        return -1;
    }
};


#endif // __QTSSERVER_H__


