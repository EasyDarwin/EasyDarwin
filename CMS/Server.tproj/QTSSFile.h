/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSFile.h

    Contains:    
                    
    
    
    
*/

#include "QTSSDictionary.h"
#include "QTSSModule.h"

#include "OSFileSource.h"
#include "EventContext.h"

class QTSSFile : public QTSSDictionary
{
    public:
    
        QTSSFile();
        virtual ~QTSSFile() {}
        
        static void     Initialize();
        
        //
        // Opening & Closing
        QTSS_Error          Open(char* inPath, QTSS_OpenFileFlags inFlags);
        void                Close();
        
        //
        // Implementation of stream functions.
        virtual QTSS_Error  Read(void* ioBuffer, UInt32 inLen, UInt32* outLen);
        
        virtual QTSS_Error  Seek(UInt64 inNewPosition);
        
        virtual QTSS_Error  Advise(UInt64 inPosition, UInt32 inAdviseSize);
        
        virtual QTSS_Error  RequestEvent(QTSS_EventType inEventMask);
        
    private:

        QTSSModule* fModule;
        UInt64      fPosition;
        QTSSFile*   fThisPtr;
        
        //
        // File attributes
        UInt64      fLength;
        time_t      fModDate;

        static QTSSAttrInfoDict::AttrInfo   sAttributes[];
};

