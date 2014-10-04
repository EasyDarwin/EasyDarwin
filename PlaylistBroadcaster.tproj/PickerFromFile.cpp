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





#include "PickerFromFile.h"


#include <stdlib.h> 
#include "GetWord.h"
#include "Trim.h"
#include "MyAssert.h"

#include <sys/stat.h>
#ifndef __Win32__
    #include <dirent.h>
#endif

#ifdef __hpux__
    #include <sys/types.h>
#endif

#define kMaxPickerPath 512

#include <stdio.h>  

static bool CompareNameToElement( PLDoubleLinkedListNode<LoopDetectionListElement>* node, void* name );
static void DisplayPickerErr( int pickErr, const char *message, const char*fname, int lineCount, const char*lineBuff );

static bool CompareNameToElement( PLDoubleLinkedListNode<LoopDetectionListElement>* node, void* name )
{
    if ( !::strcmp( node->fElement->mPathName, (const char*)name ) )
        return true;
    
    return false;
}

static bool IsDir(char* path);
static bool IsDir(char* path)
{
    struct stat data;
    
    int err = stat(path, &data);
    
    if (err == -1)
        return false;
        
    return ((data.st_mode & S_IFDIR) == S_IFDIR );
}

static void DisplayPickerErr( int pickErr, const char *message, const char*fname, int lineCount, const char*lineBuff )
{
    char *errMessage;
    
    qtss_printf( "- %s:\n", message );
    
    if ( lineCount )
        qtss_printf( "  Playlist: %s, line# %i\n", fname, lineCount );
    else
        qtss_printf( "  Playlist: %s\n", fname );
    
    if ( lineBuff )
        qtss_printf( "  Playlist text: %s", lineBuff ); // lineBuff already includes a \n
    
    switch ( pickErr )
    {
        case kPickerPopulateLoopDetected:
            errMessage = "Include would create a loop.\n";
            break;

        case kPickerPopulateBadFormat:
            errMessage = "Playlist file is missing *PLAY-LIST* identifier.\n";
            break;
            
        case kPickerPopulateFileError:
            errMessage = "Playlist file could not be opened.\n";
            break;
            
        case kPickerPopulateNoMem:
        default:
            errMessage = "Internal error occurred.\n";
            break;
    }

    qtss_printf( "  Reason: %s\n", errMessage);
}

bool PathIsAbsolute(char *pathPtr)
{
    bool result = false;
#ifdef __Win32__
    if ( (pathPtr[1] == ':') && ( pathPtr[2] == kPathDelimiterChar ) )
        result = true;
#else
    if ( *pathPtr == kPathDelimiterChar )
        result = true;
#endif
    return result;
}


int PopulatePickerFromFile( PlaylistPicker* picker, char* fname, const char* basePath, LoopDetectionList *ldList )
{
    Assert( picker );
    Assert( fname );
    
    FILE*                       weightings = NULL;
    LoopDetectionListElement*   ldElement = NULL;
    LoopDetectionNode*          ldNode = NULL;
    int                         lineCount = 0;
    int                         pickErr = kPickerPopulateNoErr; 
    char                        path[kMaxPickerPath];
    
    
        
#if kPartialPathBeginsWithDelimiter
    if (PathIsAbsolute(fname))
    {
        if ( *basePath )
            fname++;
#else
    if ( !PathIsAbsolute(fname) )
    {
#endif
        // it's a partial path, expand it to include all
        // previously traversed paths
        ::strncpy( path, basePath, kMaxPickerPath-1 );
        ::strncat( path, fname, kMaxPickerPath-1 );
            
    }
    else
    {
        // it's an absolute reference. use the path
        // part of this for the new basePath
        ::strncpy( path, fname, kMaxPickerPath-1 );
        
    }
    
    // path is now either an absolute or working directory
    // referenced partial path to the playlist file.
    int len = strlen(path);
    char lastChar = path[len-1];
    if (lastChar == '\n' || lastChar == '\r' || lastChar == ' ')
        path[len-1] = '\0';

    // ldList is passed as NULL by the initial caller.  recursive calls
    // pass along the ldList we create hre
    if ( ldList == NULL )
        ldList = new LoopDetectionList;

    Assert( ldList );
    
    if ( !ldList )
        pickErr = kPickerPopulateNoMem;
    
    
    if ( !pickErr )
    {
        if ( ldList->ForEachUntil( CompareNameToElement, path ) )
        {
            // we're already in the include chain, this is a loop
            // print a warning (error?) and continue past the loop.
            //qtss_printf("- Playlists include loop at file: %s\n", path );
            pickErr = kPickerPopulateLoopDetected;
        }
    }
    
    
    
    if ( !pickErr )
    {
        ldElement = new LoopDetectionListElement( path );
        
        Assert( ldElement );
        
        if ( ldElement )
        {   ldNode = new LoopDetectionNode( ldElement );
            Assert( ldNode );
            if ( !ldNode )
                pickErr = kPickerPopulateNoMem;
        }
        else
            pickErr = kPickerPopulateNoMem;
    }
    
    if (::IsDir(path))
        return ::PopulatePickerFromDir(picker, path);
    
    if ( !pickErr )
    {
        weightings = ::fopen( path, "r" );

        if (!weightings) 
        {
            qtss_printf("- Playlist picker failed opening list file %s\n", path);
            pickErr = kPickerPopulateFileError;
        }
    }
    
    if ( !pickErr )
    {
        SInt32  lineBuffSize = (kMaxPickerPath *2) - 1;
        SInt32  wordBuffSize = kMaxPickerPath - 1;
        
        char    lineBuff[kMaxPickerPath * 2];
        char    wordBuff[kMaxPickerPath];
        char*   next;
        char*   pathEnd;
        char*   thisLine;
        
        // add ourselves to the list
        ldList->AddNode( ldNode );
        
        // trim off the file name to get just the path part
        pathEnd = ::strrchr( path, kPathDelimiterChar );
        
        if ( pathEnd )
        {   
            pathEnd++;
            *pathEnd = 0;
        }
        else
            *path = 0;
        
        thisLine = lineBuff;
        
        if ( ::fgets( lineBuff, lineBuffSize, weightings ) != NULL )
        {
            lineCount++;
            
            thisLine = ::TrimLeft( lineBuff );
            
            if ( 0 != ::strncmp(thisLine,"*PLAY-LIST*",11) )
            {   
                //qtss_printf("- Playlist file missing *PLAY-LIST* identifier as first line:\n");
                //qtss_printf("  %s%s\n", basePath, fname);
                pickErr = kPickerPopulateBadFormat;
            }
        }
        
            
        if ( !pickErr )
        {
            do 
            {   
                next = lineBuff;
                
                if ( ::fgets( lineBuff, lineBuffSize, weightings ) == NULL )
                    break;
                
//                qtss_printf("line = %s\n", lineBuff);
                lineCount++;
                
                next = ::TrimLeft( lineBuff );
                
                if ( *next == '#' )
                {
                    // it's a comment - just toss
                    
                    //if ( *next )
                    //  qtss_printf( "comment: %s" , &lineBuff[1] );
                    
                }
                else if (*next == '+') // a list
                {
                    next = ::TrimLeft( next+1 );    // skip past + include
                    
                    if ( *next == '"' ) // get the name from the next part of the buff
                        next = ::GetQuotedWord( wordBuff, next, wordBuffSize );
                    else
                        next = ::GetWord( wordBuff, next, wordBuffSize );
                        
                    
                    
                    // recusively populate from the include file.
                    pickErr = PopulatePickerFromFile( picker, wordBuff, path, ldList );
                    
                    if ( pickErr )
                    {   
                        DisplayPickerErr( pickErr, "Playlist Include failed",  fname, lineCount, lineBuff );
                        pickErr = kPickerPopulateNoErr;
                    }
                }
                else if ( *next )
                {
                    char    numBuff[32];
                    char    expandedFileName[kMaxPickerPath];
                    int     weight = 10;    // default weight is 10

                    // get the movie file name
                    if ( *next == '"' )
                        next = ::GetQuotedWord( wordBuff, next, wordBuffSize );
                    else
                        next = ::GetWord( wordBuff, next, wordBuffSize );
                
                    if (*wordBuff)
                    {
                        #if kPartialPathBeginsWithDelimiter
                        if ( PathIsAbsolute(wordBuff) )
                        {
                            char *wordStart = wordBuff;
                            if ( *path )
                                wordStart++;
                            // full or partial path to the movie
                            ::strcpy( expandedFileName, path );
                            ::strcat( expandedFileName, wordStart );
                        }
                        #else
                        if ( !PathIsAbsolute(wordBuff) )
                        {
                            // it's a partial path..
                            
                            // cat the path and fname to form the 
                            // full or partial path to the movie
                            ::strcpy( expandedFileName, path );
                            ::strcat( expandedFileName, wordBuff );
                        }
                        #endif
                        else
                        {   // it's an absolute path..
                            ::strcpy( expandedFileName, wordBuff );
                        }
                        
                        // then get the weighting ( if supplied )
                        next = ::GetWord( numBuff, next, 32 );

                        if ( *numBuff )
                            weight = ::atoi(numBuff);

 //                       qtss_printf("expanded file name = %s\n", expandedFileName);
                        if (::IsDir(expandedFileName))
                            pickErr = ::PopulatePickerFromDir(picker, expandedFileName, weight);
                        else if ( !picker->AddToList( expandedFileName, weight ) )
                            pickErr = kPickerPopulateNoMem;
                    }
                }
                
            } while ( feof( weightings ) == 0 && pickErr == kPickerPopulateNoErr );
        }
        
        // remove ourselves from the list
        ldList->RemoveNode( ldNode );
    }
    
    // only report unreported errors.
    if ( ldList && ldList->GetNumNodes() == 0 && pickErr )
        ::DisplayPickerErr( pickErr, "Playlist error", fname, lineCount, NULL );
        
    
    if ( ldNode )
        delete ldNode; // node deletes element
    else if ( ldElement )
        delete ldElement;
    

    if ( weightings )
        (void)::fclose( weightings );

    if ( ldList && ldList->GetNumNodes() == 0 )
    {
        // all done now!
        delete ldList;
        ldList = NULL;
    
    }
    
    return pickErr;
}

int PopulatePickerFromDir( PlaylistPicker* picker, char* dirPath, int weight )
{
    static char expandedFileName[kMaxPickerPath];   // static so we don't build up the stack frame on recursion
    int pickErr = 0;
    if (dirPath != NULL)
        strcpy(expandedFileName, dirPath);
    
#ifdef __Win32__
    WIN32_FIND_DATA findData;
    HANDLE findResultHandle;
    Bool16 keepSearching = true;
    int len = strlen(expandedFileName);
    if (expandedFileName[len - 1] != kPathDelimiterChar)
    {
        expandedFileName[len] = kPathDelimiterChar;
        expandedFileName[len+1] = 0;
        len++;
    }
    strcat(expandedFileName, "*");
    
    findResultHandle = ::FindFirstFile( expandedFileName, &findData);
    if ( NULL == findResultHandle || INVALID_HANDLE_VALUE == findResultHandle )
    {
        //qtss_printf( "FindFirstFile( \"%s\" ): gle = %"_U32BITARG_"\n", searchPath, GetLastError() );
        return 0;
    }

    while ( (pickErr == 0) && keepSearching )
    {
        expandedFileName[len] = 0;  // retruncate name
        if (findData.cFileName[0] != '.')       // ignore anything beginning with a "."
        {
            strcat(expandedFileName, findData.cFileName);
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) 
                pickErr = PopulatePickerFromDir(picker, NULL, weight);
            else if ( !picker->AddToList( expandedFileName, weight ) )
                pickErr = kPickerPopulateNoMem;
        }

        keepSearching = FindNextFile( findResultHandle, &findData );
    }
        
#else
    DIR* dir;
    struct dirent* entry;
    int len = strlen(expandedFileName);
    
    if (expandedFileName[len - 1] != kPathDelimiterChar)
    {
        expandedFileName[len] = kPathDelimiterChar;
        expandedFileName[len+1] = 0;
        len++;
    }

    dir = opendir(expandedFileName);
    if (dir == NULL)
        return kPickerPopulateFileError;
        
    do {
        entry = readdir(dir);
        if (entry == NULL) break;
        
        if (entry->d_name[0] == '.')        // ignore anything beginning with a "."
            continue;

        if (len + strlen(entry->d_name) < kMaxPickerPath)
        {
            strcat(expandedFileName, entry->d_name);
            
#if __solaris__ || __sgi__ || __osf__ || __hpux__
			if (::IsDir(expandedFileName))
#else
            if ((entry->d_type & DT_DIR) != 0)
#endif
                pickErr = PopulatePickerFromDir(picker, NULL, weight);
            else if ( !picker->AddToList( expandedFileName, weight ) )
                pickErr = kPickerPopulateNoMem;
        }
        expandedFileName[len] = 0;  // retruncate name
    } while (pickErr == 0);
    
    //close the directory back up
    (void)::closedir(dir);
    
#endif  
    return pickErr;
}



