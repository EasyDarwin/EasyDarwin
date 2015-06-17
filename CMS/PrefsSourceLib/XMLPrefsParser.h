/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       XMLPrefsParser.h

    Contains:   A generic interface for pulling prefs.
*/

#ifndef __XML_PREFS_PARSER__
#define __XML_PREFS_PARSER__

#include "OSFileSource.h"
#include "OSQueue.h"
#include "StringParser.h"
#include "XMLParser.h"

typedef XMLTag* ContainerRef;

class XMLPrefsParser : public XMLParser
{
    public:
    
        XMLPrefsParser(char* inPath);
        ~XMLPrefsParser();
    
        //
        // Check for existence, man.
        
        //
        // PARSE & WRITE THE FILE. Returns true if there was an error
        int     Parse();

        // Completely replaces old prefs file. Returns true if there was an error
        int     WritePrefsFile();

        //
        // ACCESSORS

        ContainerRef    GetRefForModule( char* inModuleName, Bool16 create = true);
        
        ContainerRef    GetRefForServer();
        
        //
        // Returns the number of pref values for the pref at this index
        UInt32  GetNumPrefValues(ContainerRef pref);
        
        //
        // Returns the number of prefs associated with this given module
        UInt32  GetNumPrefsByContainer(ContainerRef container);
        
        //
        // Returns the pref value at the specfied location
        char*   GetPrefValueByIndex(ContainerRef container, const UInt32 inPrefsIndex, const UInt32 inValueIndex,
                                            char** outPrefName, char** outDataType);
                                        
        char*   GetPrefValueByRef(ContainerRef pref, const UInt32 inValueIndex,
                                            char** outPrefName, char** outDataType);
                                        
        ContainerRef    GetObjectValue(ContainerRef pref, const UInt32 inValueIndex);

        ContainerRef    GetPrefRefByName(   ContainerRef container,
                                            const char* inPrefName);
        
        ContainerRef    GetPrefRefByIndex(  ContainerRef container,
                                            const UInt32 inPrefsIndex);
        
        //
        // MODIFIERS
        
        //
        // Creates a new pref. Returns the index of that pref. If pref already
        // exists, returns existing index.
        ContainerRef    AddPref( ContainerRef container, char* inPrefName, char* inPrefDataType );

        void    ChangePrefType( ContainerRef pref, char* inNewPrefDataType);
                            
        void    AddNewObject( ContainerRef pref );

        void    AddPrefValue(   ContainerRef pref, char* inNewValue);
        
        //
        // If this value index does not exist yet, and it is one higher than
        // the highest one, this function implictly adds the new value.
        void    SetPrefValue(   ContainerRef pref, const UInt32 inValueIndex,
                                char* inNewValue);
        
        //
        // Removes the pref entirely if # of values drops to 0
        void    RemovePrefValue(    ContainerRef pref, const UInt32 inValueIndex);

        void    RemovePref( ContainerRef pref );
                
    private:
        
        XMLTag*     GetConfigurationTag();
};

#endif //__XML_PREFS_PARSER__
