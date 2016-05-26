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
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
    File:       QTSSDataConverter.h

    Contains:   Utility routines for converting to and from
                QTSS_AttrDataTypes and text
                
    Written By: Denis Serenyi

    Change History (most recent first):
    
*/

#include "QTSS.h"

class QTSSDataConverter
{
    public:
    
        //
        // This function converts a type string, eg, "UInt32" to the enum, qtssAttrDataTypeUInt32
        static QTSS_AttrDataType TypeStringToType( char* inTypeString);
        
        //
        // This function does the opposite conversion
        static char*    TypeToTypeString( QTSS_AttrDataType inType);
        
        //
        // This function converts a text-formatted value of a certain type
        // to its type. Returns: QTSS_NotEnoughSpace if the buffer provided
        // is not big enough.
        
        // String must be NULL-terminated.
        // If output value is a string, it will not be NULL-terminated
        static QTSS_Error   StringToValue(		char* inValueAsString,
                                                QTSS_AttrDataType inType,
                                                void* ioBuffer,
                                                UInt32* ioBufSize);
        
        // If value is a string, doesn't have to be NULL-terminated.
        // Output string will be NULL terminated.
        static char* ValueToString( 	    void* inValue,
                                            const UInt32 inValueLen,
                                            const QTSS_AttrDataType inType);
        

        // Takes a pointer to buffer and converts to hex in high to low order
        static char* ConvertBytesToCHexString( void* inValue, const UInt32 inValueLen);
        
        // Takes a string of hex values and converts to bytes in high to low order
        static QTSS_Error ConvertCHexStringToBytes( char* inValueAsString,
                                                    void* ioBuffer,
                                                    UInt32* ioBufSize);
};

