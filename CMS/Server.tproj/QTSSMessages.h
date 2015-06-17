/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSMessages.h

    Contains:   This global dictionary provides a central mapping from message
                names to actual text messages, stored in the provided prefs source.
                
                This allows the whole server to be easily localizeable.
    
    
    
*/

#ifndef __QTSSMESSAGES_H__
#define __QTSSMESSAGES_H__

#include "QTSS.h"
#include "QTSSDictionary.h"
#include "PrefsSource.h"

class QTSSMessages : public QTSSDictionary
{
    public:
    
        // INITIALIZE
        //
        // This function sets up the dictionary map. Must be called before instantiating
        // the first RTSPMessages object.
    
        static void Initialize();
    
        QTSSMessages(PrefsSource* inMessages);
        virtual ~QTSSMessages() {
            for (UInt32 x = 0; x < numAttrs; x++)
       	       if (attrBuffer[x] != NULL)
                 delete [] attrBuffer[x];
       	    delete [] attrBuffer;
        }
        

        //Use the standard GetAttribute method in QTSSDictionary to retrieve messages
        
    private:
	char**              attrBuffer;
	UInt32              numAttrs;
    
        enum
        {
            kNumMessages = 74 // 0 based count so it is one more than last message index number
        };
    
        static char*        sMessagesKeyStrings[];
        static char*        sMessages[];
};


#endif // __QTSSMESSAGES_H__
