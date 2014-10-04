
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


#include "MakeDir.h"

#include "PathDelimiter.h"


#if (! __MACOS__)
    #include <sys/file.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #ifndef __solaris__
        #include <sys/sysctl.h>
    #endif
    #include <sys/time.h>
#else
    #include "BogusDefs.h"
#endif

#include <string.h>
#include <stdlib.h>
#include "SafeStdLib.h"


int MakeDir(const char* inPath, int mode)
{
    struct stat theStatBuffer;
    if (stat(inPath, &theStatBuffer) == -1)
    {
        //this directory doesn't exist, so let's try to create it
        if (mkdir(inPath, mode) == -1)
            return  -1; //€- (QTSS_ErrorCode)OSThread::GetErrno();
    }
    else if (!S_ISDIR(theStatBuffer.st_mode))
        return  -1; //€- QTSS_FileExists;//there is a file at this point in the path!

    //directory exists
    return  0; //€- QTSS_NoErr;
}

int RecursiveMakeDir(const char* inPath, int mode)
{
    //PL_ASSERT(inPath != NULL);
    char    pathCopy[256];
    char*   thePathTraverser = pathCopy;
    int     theErr;
    char    oldChar;    
    
    
    if ( strlen( inPath ) > 255 )
        return -1;
    
    //iterate through the path, replacing kPathDelimiterChar with '\0' as we go
    
    strcpy( pathCopy, inPath );
    
    //skip over the first / in the path.
    if (*thePathTraverser == kPathDelimiterChar )
        thePathTraverser++;
        
    while (*thePathTraverser != '\0')
    {
        if (*thePathTraverser == kPathDelimiterChar)
        {
            //we've found a filename divider. Now that we have a complete
            //filename, see if this partial path exists.
            
            //make the partial path into a C string
            // mkdir does not like a trailing '/'
            oldChar = *thePathTraverser;
            *thePathTraverser = '\0';
            theErr = MakeDir(pathCopy, mode);
            //there is a directory here. Just continue in our traversal
            *thePathTraverser = oldChar;
            
            if (theErr)
                return theErr;
        }
        
        thePathTraverser++;
    }
    
    //need to create the last directory in the path
    return MakeDir(inPath, mode);
}
