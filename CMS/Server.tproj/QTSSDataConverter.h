/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
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

