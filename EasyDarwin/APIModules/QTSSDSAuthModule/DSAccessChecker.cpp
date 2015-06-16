/*
 *
 * Copyright (c) 1999-2005 Apple Computer, Inc.  All Rights Reserved.
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
	File:   	DSAccessChecker.cpp

	Contains:   Class definition for access checking via Open Directory
  
	Created By: Dan Sinema
  
	Created: Jan 14, 2005
  
*/

/*
 *	Directory Service code added by Dan Sinema
 *	
 *	Jan 14, 2005 - Cleaned up code and added more comments.
 *	Nov 8, 2004 - Finsihed final code. Added group support.
 *	
*/


// ANSI / POSIX Headers
#include <grp.h>
#include <membership.h>
#include <pwd.h>
#include <signal.h>
#include <unistd.h>

// STL Headers
#include <cstdio>
#include <cstdlib>
#include <cstring>


// Project Headers
#include "SafeStdLib.h"	
#include "StrPtrLen.h"
#include "StringParser.h"
#include "ResizeableStringFormatter.h"

#include "DSAccessChecker.h"
#include "DSDataList.h"
#include "QTAccessFile.h"

#define DEBUG_DSACCESS 0
#define debug_printf if (DEBUG_DSACCESS) ::qtss_printf

#include <AvailabilityMacros.h>

#ifdef AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER
	#if OSX_OD_API
		#define OD_API 1
		#define DS_API 0 
	#else
	    #define OD_API 0
		#define DS_API 1
	#endif
#else
    #define OD_API 0
    #define DS_API 1
#endif

// Framework Headers
#if DS_API
#include <DirectoryService/DirectoryService.h>
using namespace DirectoryServices;
#endif

#if OD_API
#include <OpenDirectory/OpenDirectory.h>
#endif

#if __LP64__
   #define ds_API_PTR UInt32*
#else
   #define ds_API_PTR long unsigned int*
#endif


#pragma mark DSAccessChecker class globals

const char* DSAccessChecker::kDefaultAccessFileName = "qtaccess";


#pragma mark DSAccessChecker class implementation
#if DS_API

// Find a list of records that match the given criteria.
static SInt32 _GetRecordList(
	tDirReference inDSRef,
	const char *inDomain,
	const char *inRecName,
	const char *inRecType,
	tDataList *inAttrType,
	tDirNodeReference *outNodeRef,
	tDataBuffer **outDataBuff,
	UInt32 *outRecCount )
{
	SInt32					 	status				= eDSNoErr;
	tDataBuffer					*pDataBuff			= NULL;
	tDirNodeReference			nodeRef				= 0;
	tContextData				context				= NULL;
	UInt32						nodeCount			= 0;
	tDataList					*nodeName			= NULL;
	UInt32						recCount			= 0;

	*outNodeRef = 0;
	*outDataBuff = NULL;
	*outRecCount = 0;

	pDataBuff = ::dsDataBufferAllocate( inDSRef, 4096 );
	if (pDataBuff == NULL)
	{   
		// We need the buffer for locating the node for which the user object resides
		debug_printf("QTSSODAuthModule: Unable to allocate buffer.\n");
		return eDSAllocationFailed;
	}

		// This is the default action with no domain: use the Search node.
	status = ::dsFindDirNodes( inDSRef, pDataBuff, NULL, eDSSearchNodeName, (ds_API_PTR) &nodeCount, &context );
	if ( context != NULL )
	{
		::dsReleaseContinueData( inDSRef, context );
		context = NULL;
	}

	// Check for failure of the dsFindDirNodes
	// Node count less than 1 means no node found...doh! 
	if ( nodeCount < 1 )
	{
		status = eDSNodeNotFound;
	}
	if ( status != eDSNoErr )
	{
		goto cleanupBadGetRecordList;
	}

	// Extract the name of the found node.
	status = ::dsGetDirNodeName( inDSRef, pDataBuff, 1, &nodeName );
	if (status == eDSNoErr)
	{
		// Open the node so we can do the DS magic
		status = ::dsOpenDirNode( inDSRef, nodeName, &nodeRef );
		::dsDataListDeallocate( inDSRef, nodeName );
		std::free( nodeName );
		nodeName = NULL;
	}

	if (status != eDSNoErr)
	{	
		// Bail if we cannot open the node.
		debug_printf("QTSSODAuthModule: Could not open node - error: %"_S32BITARG_"\n",  status);
	}
	else
	{
		// Specify what we are looking for...
		// pRecName: the passed name of the record
		// pRecType: the passed name of the record type
		// pAttrType: attributes to return to the caller
		DSDataList recName( inDSRef, inRecName );
		DSDataList recType( inDSRef, inRecType );
		
		recCount = 1;
		
		// Find the record that matchs the above criteria
		status = ::dsGetRecordList( nodeRef, pDataBuff, recName, eDSExact, recType, inAttrType, 0, (ds_API_PTR)&recCount, &context );
		if ( context != NULL )
		{
			::dsReleaseContinueData( inDSRef, context );
			context = NULL;
		}
		if ( recCount == 0 )
		{
			status = eDSRecordNotFound;
			debug_printf("QTSSODAuthModule: No records found.\n");
		}
		else if ( status != eDSNoErr )
		{
			debug_printf("QTSSODAuthModule: No records found - error: %"_S32BITARG_"\n",  status);
		}
	}
	
	if ( status == eDSNoErr )
	{
		*outNodeRef = nodeRef;
		*outDataBuff = pDataBuff;
		*outRecCount = recCount;
		return eDSNoErr;
	}

cleanupBadGetRecordList:
	if ( nodeRef != 0 )
	{
		::dsCloseDirNode( nodeRef );
	}

	// This variable is guaranteed to be valid because the function would
	// have returned if it was bad.
	::dsDataBufferDeAllocate( inDSRef, pDataBuff );

	return status;
}

static SInt32 _FindRecordNode(
	tDirReference inDSRef,
	const char *inDomain,
	const char *inRecName,
	const char *inRecType,
	tDataList *outHomeNodeName )
{
	tDataBuffer					*pRecBuff			= NULL;
	tDirNodeReference			nodeRef				= 0;
	SInt32					 	status				= eDSNoErr;
	UInt32				attrIndex			= 0;
	UInt32				recCount			= 0;
	tRecordEntry		  	 	*pRecEntry			= NULL;
	tAttributeListRef			attrListRef			= 0;

	if ( outHomeNodeName == NULL )
	{
		return eDSNullDataList;
	}
	std::memset( outHomeNodeName, 0, sizeof( *outHomeNodeName) );

	// A Username and Password is needed, if either one is not present then bail!
	if ( inRecName == NULL )
	{
		return eDSInvalidRecordName;
	}
	if ( inRecType == NULL )
	{
		return eDSInvalidRecordType;
	}

	status = ::_GetRecordList( inDSRef, inDomain, inRecName, inRecType,
								DSDataList (inDSRef, kDSNAttrMetaNodeLocation),
								&nodeRef, &pRecBuff, &recCount );
	if ( status != eDSNoErr )
	{
		return status;
	}

	// Get the record entry out of the list, there should only be one record!
	status = ::dsGetRecordEntry( nodeRef, pRecBuff, 1, &attrListRef, &pRecEntry );
	if ( status != eDSNoErr )
	{
		// These variables are guaranteed to be valid because the function would
		// have returned if they were bad.
		::dsCloseDirNode( nodeRef );
		::dsDataBufferDeAllocate( inDSRef, pRecBuff );
		return status;
	}

	// Now loop through attributes of the entry...looking for kDSNAttrMetaNodeLocation and kDSNAttrRecordName
	for ( attrIndex = 1; (attrIndex <= pRecEntry->fRecordAttributeCount) && (status == eDSNoErr); attrIndex++ )
	{
		tAttributeEntryPtr		pAttrEntry	= NULL;
		tAttributeValueListRef	valueRef	= 0;

		status = ::dsGetAttributeEntry( nodeRef, pRecBuff, attrListRef, attrIndex, &valueRef, &pAttrEntry );
		if ( ( status != eDSNoErr ) || ( pAttrEntry == NULL ) )
			continue;
		// Test for kDSNAttrMetaNodeLocation
		if ( std::strcmp( pAttrEntry->fAttributeSignature.fBufferData, kDSNAttrMetaNodeLocation ) == 0 )
		{
			tAttributeValueEntry	*pValueEntry	= NULL;

			// If it matches then get the value of the attribute
			status = ::dsGetAttributeValue( nodeRef, pRecBuff, 1, valueRef, &pValueEntry );
			if ( ( status == eDSNoErr ) && ( pValueEntry != NULL ) )
			{
				// Store the node location in outHomeNodeName
				if ( outHomeNodeName->fDataNodeCount != 0 )
				{
					debug_printf("QTSSODAuthModule: Multiple user locations found!?\n");
				}
				else
				{
					status = ::dsBuildListFromPathAlloc( inDSRef, outHomeNodeName, pValueEntry->fAttributeValueData.fBufferData, "/" );
					::dsDeallocAttributeValueEntry( inDSRef, pValueEntry );
				}
			}
		}

		::dsDeallocAttributeEntry( inDSRef, pAttrEntry );
		::dsCloseAttributeValueList( valueRef );
	}

	// Cleanup dsGetRecordEntry() return values.
	::dsCloseAttributeList( attrListRef );
	::dsDeallocRecordEntry( inDSRef, pRecEntry );
	::dsCloseDirNode( nodeRef );
	::dsDataBufferDeAllocate( inDSRef, pRecBuff );
	return status;
}

#endif

#pragma mark -
#pragma mark "Public Methods"
// Now the class proper.
DSAccessChecker::DSAccessChecker()
{
}

DSAccessChecker::~DSAccessChecker()
{
#if DEBUG
	debug_printf("QTSSODAuthModule: Access checker object destroyed.\n");
#endif
}



#if 0 //OD_API notes
/* 
    This is Leopard or later code so some check before using OD based code is needed.
    Implement this api for digest auth.
*/

#include <OpenDirectory/OpenDirectoryPriv.h>
/System/Library/PrivateFrameworks/OpenDirectory.framework/Frameworks/CFOpenDirectory.framework/CFOpenDirectory
CFErrorRef      outError = NULL;
ODNodeRef  cfNode = ODNodeCreateWithNodeType( kCFAllocatorDefault, kODSessionDefault, kODTypeAuthenticationSearchNode, NULL );
if (cfNode)
{
        ODRecordRef cfUserRecord = ODNodeCopyRecord( kCFAllocatorDefault, cfNode, CFSTR("username"), NULL );

        if (cfUserRecord != NULL)
        {
                CFArrayRef      authItems = CFArrayCreate.... ( username, server challenge, client response, http method);

                // for DIGEST_MD5
                if (ODRecordVerifyPasswordExtended( cfUserRecord, CFSTR(kDSStdAuthDIGEST_MD5), authItems, NULL, NULL, &outError ))
                {
                }

                // this for password
                if (ODRecordVerifyPassword( cfUserRecord, CFSTR("password") ) )
                {
                }
                CFRelease( cfUserRecord );
                CFRelease( autItems );
        }
        CFRelease( cfNode );
}


// kDSStdAuthDIGEST_MD5
 *     user name in UTF8 encoding,
 *     server challenge in UTF8 encoding,
 *     client response data,
 *     HTTP method in UTF8 encoding


        }
}
#endif


#if OD_API
Bool16 DSAccessChecker::CheckPassword(const char* inUsername, const char* inPassword)
{
	Bool16     checkedResult = false;
    CFErrorRef outError = NULL;
    debug_printf("DSAccessChecker::CheckPassword userName=%s password=%s\n", inUsername,inPassword);
    
    ODNodeRef  cfNodeRef= ODNodeCreateWithNodeType( kCFAllocatorDefault, kODSessionDefault, kODTypeAuthenticationSearchNode, NULL );

    //static ODRecordRef _ODNodeCopyRecord( ODNodeRef inNodeRef, CFStringRef inRecordType, CFStringRef inRecordName, CFArrayRef inAttributes, CFErrorRef *outError );


    CFStringRef cfPassword = CFStringCreateWithCString(kCFAllocatorDefault, inPassword, kCFStringEncodingUTF8);
    CFStringRef cfUsername = CFStringCreateWithCString(kCFAllocatorDefault, inUsername, kCFStringEncodingUTF8);

    CFTypeRef vals[] = { CFSTR(kDSAttributesStandardAll) };
    CFArrayRef reqAttrs = CFArrayCreate(NULL, vals,1, &kCFTypeArrayCallBacks);
    
    ODRecordRef cfUserRecord = ODNodeCopyRecord(cfNodeRef,  CFSTR(kDSStdRecordTypeUsers), cfUsername, reqAttrs,  &outError );
    if (cfNodeRef && cfUserRecord && cfPassword && cfUsername)
    {
        // this for password
        if ( ODRecordVerifyPassword( cfUserRecord, cfPassword , NULL ) )
        {   checkedResult = true;
            debug_printf("DSAccessChecker::CheckPassword ODRecordVerifyPassword user is authenticated\n");
        }
        else
        {   debug_printf("DSAccessChecker::CheckPassword ODRecordVerifyPassword user failed to authenticate\n");
        }
    }
    
    if (reqAttrs)       CFRelease( reqAttrs );
    if (cfUserRecord)   CFRelease( cfUserRecord );
    if (cfPassword)     CFRelease( cfPassword );
    if (cfUsername)     CFRelease( cfUsername );
    if (cfNodeRef)      CFRelease( cfNodeRef );
    
    return checkedResult;
	
}


Bool16 DSAccessChecker::CheckDigest(const char* inUsername, const char* inServerChallenge, const char* inClientResponse, const char* inMethod)
{
	Bool16     checkedResult = false;
    CFErrorRef outError = NULL;
    CFArrayRef outItems = NULL;
        
    if (NULL == inUsername || NULL == inServerChallenge || NULL == inClientResponse )
        return false;
        
    ResizeableStringFormatter challengeString; 	
    challengeString.Put((char*) inServerChallenge);
    challengeString.PutTerminator(); 
    char* challengeCString= challengeString.GetBufPtr();
    
    ResizeableStringFormatter responseString;
    responseString.Put( (char*)inClientResponse);
    responseString.PutTerminator(); 
    char* responseCString= responseString.GetBufPtr();

    ODNodeRef  cfNode = ODNodeCreateWithNodeType( kCFAllocatorDefault, kODSessionDefault, kODTypeAuthenticationSearchNode, NULL );
    debug_printf("DSAccessChecker::CheckDigest \nuserName=[%s] \nchallenge=[%s] \nresponse=[%s] \nmethod=[%s]\n", inUsername,challengeCString, responseCString,inMethod);
    
    
    CFTypeRef vals[] = { CFSTR(kDSAttributesStandardAll) };
    CFArrayRef reqAttrs = CFArrayCreate(NULL, vals,1, &kCFTypeArrayCallBacks);

    CFStringRef cfUsername = CFStringCreateWithCString(kCFAllocatorDefault, inUsername, kCFStringEncodingUTF8);
    ODRecordRef cfUserRecord = ODNodeCopyRecord(cfNode,  CFSTR(kDSStdRecordTypeUsers), cfUsername, reqAttrs,  &outError );
    CFRelease( cfUsername );
    cfUsername = NULL;
    
    enum { kNumAuthValues=4 };
    CFStringRef cfStringArray[kNumAuthValues];
    cfStringArray[0] = CFStringCreateWithCString(kCFAllocatorDefault, inUsername, kCFStringEncodingUTF8);
    cfStringArray[1] = CFStringCreateWithCString(kCFAllocatorDefault, challengeCString, kCFStringEncodingUTF8);
    cfStringArray[2] = CFStringCreateWithCString(kCFAllocatorDefault, responseCString, kCFStringEncodingUTF8);
    cfStringArray[3] = CFStringCreateWithCString(kCFAllocatorDefault, inMethod, kCFStringEncodingUTF8);
    
    CFArrayRef    cfAuthItems = CFArrayCreate(kCFAllocatorDefault, (const void **) &cfStringArray,kNumAuthValues, &kCFTypeArrayCallBacks);
    
    if (cfNode && cfUserRecord && cfAuthItems)
    {
        debug_printf("DSAccessChecker::CheckDigest call ODRecordVerifyPasswordExtended\n");
        // for DIGEST_MD5
        if (ODRecordVerifyPasswordExtended( cfUserRecord, CFSTR(kDSStdAuthDIGEST_MD5), cfAuthItems, &outItems, NULL, &outError ))
        {
            checkedResult = true;
            debug_printf("DSAccessChecker::CheckDigest SUCCESS ODRecordVerifyPasswordExtended response=%d\n",outError);
        }
        else
        {   debug_printf("DSAccessChecker::CheckDigest ODRecordVerifyPasswordExtended response=%d\n", outError ? CFErrorGetCode(outError) : -1);
        }
    }
    
    for (int i = 0; i < kNumAuthValues; i++)
        CFRelease(cfStringArray[i]);
        
    if (reqAttrs)       CFRelease( reqAttrs );
    if (cfAuthItems)    CFRelease( cfAuthItems );
    if (cfUserRecord)   CFRelease( cfUserRecord );
    if (cfNode)         CFRelease( cfNode );
    if (outItems)       CFRelease( outItems );
    if (outError)       CFRelease( outError );
    
    return checkedResult;
	
}



#endif

#if DS_API

Bool16 DSAccessChecker::CheckPassword(const char* inUsername, const char* inPassword)
{
	tDirReference				dsRef		= 0;
	tDataList					userNode	= { 0, NULL };
	tDirNodeReference			nodeRef		= 0;
	SInt32					 	status		= eDSNoErr;

	// A Username and Password is needed, if either one is not present then bail!
	if ( inUsername == NULL )
	{
		debug_printf("QTSSODAuthModule: Username required.\n");
		return false;
	}
	if ( inPassword == NULL )
	{
		debug_printf("QTSSODAuthModule: Password required.\n");
		return false;
	}
	status = ::dsOpenDirService( &dsRef );
	if ( status != eDSNoErr )
	{	
		// Some DS error, tell the admin what the error is and bail.
		// Error can be found in DirectoryService man page.
		debug_printf("QTSSODAuthModule: Could not open Directory Services - error: %"_S32BITARG_"", status);
		return false;
	}

	status = _FindRecordNode( dsRef, NULL, inUsername, kDSStdRecordTypeUsers, &userNode );
	if ( status != eDSNoErr )
	{
		debug_printf("QTSSODAuthModule: Could not find user node.\n");
		return false;
	}

	// Now that we know the node location of the user object, lets open that node.
	status = ::dsOpenDirNode( dsRef, &userNode, &nodeRef );
	::dsDataListDeallocate( dsRef, &userNode );

	if ( status == eDSNoErr )
	{
		UInt32	uiLen		= std::strlen( inUsername );
		tDataNode		*pAuthType	= ::dsDataNodeAllocateString( dsRef, kDSStdAuthNodeNativeClearTextOK );
		tDataBuffer		*pStepBuff	= ::dsDataBufferAllocate( dsRef, 128 );
		tDataBuffer		*pAuthBuff	= ::dsDataBufferAllocate( dsRef, ( sizeof( UInt32 ) + sizeof( UInt32 ) + uiLen + std::strlen( inPassword ) ) );

		if ( ( pStepBuff != NULL ) && ( pAuthType != NULL ) && ( pAuthBuff != NULL ) )
		{
			UInt32	uiCurr	= 0;

			// Copy username (that is passed into this function) into buffer for dsDoDirNodeAuth()
			std::memcpy( &(pAuthBuff->fBufferData[ uiCurr ]), &uiLen, sizeof( UInt32 ) );
			uiCurr += sizeof( UInt32 );
			std::memcpy( &(pAuthBuff->fBufferData[ uiCurr ]), inUsername, uiLen );
			uiCurr += uiLen;

			// Copy password into a buffer for dsDoDirNodeAuth()
			uiLen = std::strlen( inPassword );
			std::memcpy( &(pAuthBuff->fBufferData[ uiCurr ]), &uiLen, sizeof( UInt32 ) );
			uiCurr += sizeof( UInt32 );
			std::memcpy( &(pAuthBuff->fBufferData[ uiCurr ]), inPassword, uiLen );
			uiCurr += uiLen;

			pAuthBuff->fBufferLength = uiCurr;

			// Perform the authentication
			status = ::dsDoDirNodeAuth( nodeRef, pAuthType, 1, pAuthBuff, pStepBuff, NULL );
			// Since the buffer held a name & password, clear it immediately.
			std::memset( pAuthBuff, 0, pAuthBuff->fBufferSize );
		}
		// Free the auth buffer.
		if ( pAuthBuff != NULL )
		{
			::dsDataBufferDeAllocate( dsRef, pAuthBuff );
		}
		// Free the ignored step buffer.
		if ( pStepBuff != NULL )
		{
			::dsDataBufferDeAllocate( dsRef, pStepBuff );
		}
		// Free the auth string.
		if ( pAuthType != NULL )
		{
			::dsDataNodeDeAllocate( dsRef, pAuthType );
		}
		::dsCloseDirNode( nodeRef );
	}
	::dsCloseDirService( dsRef );

	if( status == eDSNoErr )
	{
		debug_printf("QTSSODAuthModule: Authentication is good.\n");
		return true;
	}

	// For admins running QTSS in debug
	debug_printf("QTSSODAuthModule: OD returned %"_S32BITARG_" status.\n", status);
	debug_printf("QTSSODAuthModule: Authentication failed.\n");
	// If the Authentication failed then return false, which boots the user...
	return false;
}

Bool16 DSAccessChecker::CheckDigest(const char* inUsername, const char* inServerChallenge, const char* inClientResponse, const char* inMethod)
{
  return false;
}

#endif

#pragma mark -
#pragma mark "Protected Methods"

Bool16 DSAccessChecker::CheckGroupMembership(const char* inUsername, const char* inGroupName)
{   
	// In Tiger, group membership is painfully simple: we ask memberd for it!
	struct passwd	*user		= NULL;
	struct group	*group		= NULL;
	uuid_t			userID;
	uuid_t			groupID;
	int				isMember	= 0;

	// Look up the user using the POSIX APIs: only care about the UID.
	user = getpwnam(inUsername);
	endpwent();
	if ( user == NULL )
		return false;
	uuid_clear(userID);
	if ( mbr_uid_to_uuid(user->pw_uid, userID) )
		return false;

	// Look up the group using the POSIX APIs: only care about the GID.
	group = getgrnam(inGroupName);
	endgrent();
	if ( group == NULL )
		return false;
	uuid_clear(groupID);
	if ( mbr_gid_to_uuid(group->gr_gid, groupID) )
		return false;

	// mbr_check_membership() returns 0 on success and error code on failure.
	if ( mbr_check_membership(userID, groupID, &isMember) )
		return false;
	return (bool)isMember;
}


