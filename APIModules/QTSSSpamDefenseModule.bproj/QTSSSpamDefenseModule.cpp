/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 */
/*
    File:       QTSSSpamDefanseModule.cpp

    Contains:   Implementation of module described in .h file

    

*/

#include "QTSSSpamDefenseModule.h"
#include "OSHashTable.h"
#include "OSMutex.h"
#include "QTSSModuleUtils.h"
#include "OSMemory.h"

static QTSS_ModulePrefsObject sPrefs = NULL;
static QTSS_StreamRef           sErrorLogStream = NULL;

class IPAddrTableKey;

class IPAddrTableElem
{
    public:

        IPAddrTableElem(UInt32 inIPAddr) : fIPAddr(inIPAddr), fRefCount(0), fNextHashEntry(NULL) {}
        ~IPAddrTableElem() {}
        
        UInt32 GetRefCount() { return fRefCount; }
        void    IncrementRefCount() { fRefCount++; }
        void    DecrementRefCount() { fRefCount--; }
    private:
        
        UInt32              fIPAddr;// this also serves as the hash value
        UInt32              fRefCount;
                
        IPAddrTableElem*    fNextHashEntry;
        
        friend class IPAddrTableKey;
        friend class OSHashTable<IPAddrTableElem, IPAddrTableKey>;
};


class IPAddrTableKey
{
public:

    //CONSTRUCTOR / DESTRUCTOR:
    IPAddrTableKey(UInt32 inIPAddr) : fIPAddr(inIPAddr) {}
    ~IPAddrTableKey() {}
    
    
private:

    //PRIVATE ACCESSORS:    
    SInt32      GetHashKey()        { return fIPAddr; }

    //these functions are only used by the hash table itself. This constructor
    //will break the "Set" functions.
    IPAddrTableKey(IPAddrTableElem *elem) : fIPAddr(elem->fIPAddr) {}
                                    
    friend int operator ==(const IPAddrTableKey &key1, const IPAddrTableKey &key2)
    {
        return (key1.fIPAddr == key2.fIPAddr);
    }
    
    //data:
    UInt32  fIPAddr;

    friend class OSHashTable<IPAddrTableElem, IPAddrTableKey>;
};

typedef OSHashTable<IPAddrTableElem, IPAddrTableKey> IPAddrHashTable;

// STATIC DATA
static IPAddrHashTable*         sHashTable = NULL;
static OSMutex*                 sMutex;
static UInt32                   sNumConnsPerIP = 0;
static UInt32                   sDefaultNumConnsPerIP = 100;

// ATTRIBUTES
static QTSS_AttributeID         sIsFirstRequestAttr = qtssIllegalAttrID;
static QTSS_AttributeID         sTooManyConnectionsErr = qtssIllegalAttrID;

// FUNCTION PROTOTYPES
static QTSS_Error   QTSSSpamDefenseModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs();
static QTSS_Error Authorize(QTSS_StandardRTSP_Params* inParams);
static QTSS_Error SessionClosing(QTSS_RTSPSession_Params* inParams);

// FUNCTION IMPLEMENTATIONS


QTSS_Error QTSSSpamDefenseModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSSpamDefenseModuleDispatch);
}


QTSS_Error  QTSSSpamDefenseModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
        case QTSS_RTSPAuthorize_Role:
            return Authorize(&inParams->rtspAuthParams);
        case QTSS_RTSPSessionClosing_Role:
            return SessionClosing(&inParams->rtspSessionClosingParams);
    }
    return QTSS_NoErr;
}

QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // The spam defense module has one preference, the number of connections
    // to allow per ip addr
    static char*        sIsFirstRequestName = "QTSSSpamDefenseModuleIsFirstRequest";

    // Add text messages attributes
    static char*        sTooManyConnectionsName = "QTSSSpamDefenseModuleTooManyConnections";

    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
    
    (void)QTSS_AddRole(QTSS_RTSPAuthorize_Role);
    (void)QTSS_AddRole(QTSS_RTSPSessionClosing_Role);
    
    (void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sIsFirstRequestName, NULL, qtssAttrDataTypeBool16);
    (void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sIsFirstRequestName, &sIsFirstRequestAttr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sTooManyConnectionsName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sTooManyConnectionsName, &sTooManyConnectionsErr);

    // Tell the server our name!
    static char* sModuleName = "QTSSSpamDefenseModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
	sErrorLogStream = inParams->inErrorLogStream;
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);
    sMutex = NEW OSMutex();
    sHashTable = NEW IPAddrHashTable(277);//277 is prime, I think...
    RereadPrefs();
    return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
    QTSSModuleUtils::GetAttribute(sPrefs, "num_conns_per_ip_addr", qtssAttrDataTypeUInt32,
                                &sNumConnsPerIP, &sDefaultNumConnsPerIP, sizeof(sNumConnsPerIP));
    return QTSS_NoErr;
}

QTSS_Error Authorize(QTSS_StandardRTSP_Params* inParams)
{
    static Bool16 sTrue = true;
    
    Bool16* isFirstRequest = NULL;
    UInt32* theIPAddr = NULL;
    UInt32 theLen = 0;
    
    // Only do anything if this is the first request
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, sIsFirstRequestAttr, 0, (void**)&isFirstRequest, &theLen);
    if (isFirstRequest != NULL)
        return QTSS_NoErr;
        
    // Get the IP address of this client.
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, qtssRTSPSesRemoteAddr, 0, (void**)&theIPAddr, &theLen);
    if ((theIPAddr == NULL) || (theLen != sizeof(UInt32)))
    {
        return QTSS_NoErr;
    }

    IPAddrTableKey theKey(*theIPAddr);
    
    // This must be atomic
    OSMutexLocker locker(sMutex);

    // Check to see if this client currently has a connection open.
    IPAddrTableElem* theElem = sHashTable->Map(&theKey);
    if (theElem == NULL)
    {
        // Client doesn't have a connetion open currently. Create a map element,
        // and add it into the map.
        theElem = NEW IPAddrTableElem(*theIPAddr);
        sHashTable->Add(theElem);
    }
    
    // Check to see if this client has too many connections open. If it does,
    // return an error, otherwise, allow the connection and increment the
    // refcount.
    if (theElem->GetRefCount() >= sNumConnsPerIP) {
		QTSSModuleUtils::LogErrorStr(qtssMessageVerbosity, "Blocking connection from IP address");
        return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientForbidden,
                                                    sTooManyConnectionsErr);
    } else
        theElem->IncrementRefCount();
        
    // Mark the request so we'll know subsequent ones aren't the first.
    // Note that we only do this if we've successfully added this client to our map.
    // That way, we only remove it in SessionClosing if we've added it.
    (void)QTSS_SetValue(inParams->inRTSPSession, sIsFirstRequestAttr, 0, &sTrue, sizeof(sTrue));
    
    return QTSS_NoErr;
}

QTSS_Error SessionClosing(QTSS_RTSPSession_Params* inParams)
{
    UInt32* theIPAddr = NULL;
    Bool16* isFirstRequest = NULL;
    UInt32 theLen = 0;
    
    // Only remove this session from the map if it has been added in the first place
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, sIsFirstRequestAttr, 0, (void**)&isFirstRequest, &theLen);
    if (isFirstRequest == NULL)
        return QTSS_NoErr;
        
    // Get the IP address of this client.
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, qtssRTSPSesRemoteAddr, 0, (void**)&theIPAddr, &theLen);
    if ((theIPAddr == NULL) || (theLen != sizeof(UInt32)))
    {
        return QTSS_NoErr;
    }

    IPAddrTableKey theKey(*theIPAddr);
    
    // This must be atomic
    OSMutexLocker locker(sMutex);

    // Check to see if this client currently has a connection open.
    IPAddrTableElem* theElem = sHashTable->Map(&theKey);
    if (theElem == NULL)
        return QTSS_NoErr; //this may happen if there is another module denying connections
        
    // Decrement the refcount
    if (theElem->GetRefCount() > 0)
        theElem->DecrementRefCount();
    
    // If the refcount is 0, remove this from the map, and delete it.
    if (theElem->GetRefCount() == 0)
    {
        sHashTable->Remove(theElem);
        delete theElem;
    }
    
    return QTSS_NoErr;      
}
