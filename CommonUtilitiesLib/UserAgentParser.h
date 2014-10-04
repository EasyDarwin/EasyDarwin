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
    File:       UserAgentParser.h

    Contains:   API interface for parsing the user agent field received from RTSP clients.
                
    Change History (most recent first):

    
    
    
*/
#ifndef _USERAGENTPARSER_H_
#define _USERAGENTPARSER_H_

#include "StringParser.h"
#include "StringFormatter.h"
#include "StrPtrLen.h"

class UserAgentParser 
{
    public:
        enum{   eMaxAttributeSize   =  60 };
        struct UserAgentFields
        {
            char                    fFieldName[eMaxAttributeSize + 1];
            UInt32                  fLen;
            UInt32                  fID;
        };

        struct UserAgentData
        {           
            StrPtrLen               fData;
            bool                    fFound;
        };

        enum 
        {   eQtid   = 0,
            eQtver  = 1,
            eLang   = 2,
            eOs     = 3,
            eOsver  = 4,
            eCpu    = 5,
            eNumAttributes = 6 
        };

        static UserAgentFields sFieldIDs[];
        static UInt8 sEOLWhitespaceEqualMask[];
        static UInt8 sEOLSemicolonCloseParenMask[];
        static UInt8 sWhitespaceMask[];

        UserAgentData fFieldData[eNumAttributes];
            
        void Parse(StrPtrLen *inStream);

        StrPtrLen* GetUserID()          { return    &(fFieldData[eQtid].fData);     };
        StrPtrLen* GetUserVersion()     { return    &(fFieldData[eQtver].fData);    };
        StrPtrLen* GetUserLanguage()    { return    &(fFieldData[eLang].fData);     };
        StrPtrLen* GetrUserOS()         { return    &(fFieldData[eOs].fData);       };
        StrPtrLen* GetUserOSVersion()   { return    &(fFieldData[eOsver].fData);    };
        StrPtrLen* GetUserCPU()         { return    &(fFieldData[eCpu].fData);      };
        
        UserAgentParser (StrPtrLen *inStream)  { if (inStream != NULL) Parse(inStream); }


};


#endif // _USERAGENTPARSER_H_
