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
 * util.c
 *
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <assert.h>

#include "util.h"
static char to_lower(int c);

/**********************************************/
static char ip_string_buffer[20];
char *ip_to_string(int ip) {
    sprintf(ip_string_buffer, "%d.%d.%d.%d",
        (ip & 0xff000000) >> 24, (ip & 0x00ff0000) >> 16,
        (ip & 0x0000ff00) >> 8, (ip & 0x000000ff));
    return ip_string_buffer;
}

/**********************************************/
static char to_lower(int c)
{
    if (c >= 'A' && c <= 'Z')
        return ((c - 'A') + 'a');
    return c;
}

/**********************************************/
char *str_dup(char *str)
{
    char *ret;
    
    ret = (char*)malloc(strlen(str)+1);
    strcpy(ret, str);
    return ret;
}

/**********************************************/
int str_casecmp(char *str1, char *str2)
{
    int ret;
    
    ret = *str1 - *str2;
    while (*str1 && *str2 && ((ret = *str1++ - *str2++) == 0))
        ;
    return ret;
}

/**********************************************/
int strn_casecmp(char *str1, char *str2, int l)
{
    int ret;
    
    ret = to_lower( (char) *str1) - to_lower((char) *str2);
    while (l-- && to_lower(*str1) && to_lower(*str2) && ((ret = to_lower(*str1++) - to_lower(*str2++)) == 0))
        ;
    return ret;
}

/*
    grab a full line from input
    put into strBuff as 0 terminated
    must maintain the exact eol that the 
    input string has.
    
    return next position.
*/

char* get_line_str( char* strBuff, char *input, int buffSize)
{
    char  *p = input;
    int     sawEOLChar = 0;
        
        assert( strBuff != NULL && input != NULL && buffSize > 0);
    
        memset(strBuff, 0, (size_t) buffSize);
    while( *p )
    {
        assert( buffSize > 0 );
        
        if ( *p == '\r' ||  *p == '\n' )
        {   // grab all eol chars
            sawEOLChar = 1;
            *strBuff = *p;
            strBuff++;
            buffSize--;
        
        }
        else if ( sawEOLChar )
        {
            // we saw eol char(s) and now this is not one
            // that means we're done
            break;
        }
        else
        {   
            // grab all line chars
            *strBuff = *p;
            strBuff++;
            buffSize--;
            
        }
    
        p++;
    }
    
    *strBuff = 0;
    
    // we're not going to change the contents
    // of input, but the caller may have the right too.
    return (char*)p;

}


/**********************************************

    subdivide a string along the delimeter
    points in delim
    
    
    return the next start point in stringp
    
    return the current string start;

*/

char *str_sep(char **stringp, char *delim)
{
    int j, dl, i, sl;
    char *newstring, *ret;

    if (*stringp == NULL)
        return NULL;

    dl = strlen(delim);
    sl = strlen(*stringp);
    newstring = NULL;
    ret = *stringp;

    for (i=0; i<sl; i++) 
    {
        for (j=0; j<dl; j++) 
        {
            if ((*stringp)[i] == delim[j]) 
            {
                (*stringp)[i] = '\0';
                newstring = &((*stringp)[i+1]);
                i = sl; j = dl;
            }
        }
    }

    *stringp = newstring;
    return ret;
}

/**********************************************/
typedef struct t_ip_cache {
    struct t_ip_cache *next;
    char    *name;
    int     ip;
} t_ip_cache;
static t_ip_cache *gIPcache = NULL;
int check_IP_cache(char *name, int *ip)
{
    t_ip_cache *cur = gIPcache;
    
    while (cur) {
        if (str_casecmp(name, cur->name) == 0) {
            *ip = cur->ip;
            return 0;
        }
        cur = cur->next;
    }
    return -1;
}

/**********************************************/
int add_to_IP_cache(char *name, int ip)
{
    t_ip_cache *cur;
    
    cur = (t_ip_cache*)malloc(sizeof(t_ip_cache));
    if (cur == NULL)
        return -1;
    cur->ip = ip;
    cur->name = malloc(strlen(name) + 1);
    strcpy(cur->name, name);
    cur->next = gIPcache;
    gIPcache = cur;
    return 0;
}

/**********************************************/
int inet_aton_(char *s, int *retval)
{
    int i, l, x, el[4], ret, good = 1;

    x = 0, ret = 0;
    l = strlen(s);
    el[0] = 0;
    for (i=0; i<l; i++) {
        if (s[i] == '.') {
            x++;
            if (x > 3) {
                good = 0;
                break;
            }
            el[x] = 0;
        }
        else if (s[i] >= '0' && s[i] <= '9') {
            el[x] *= 10;
            el[x] += s[i] - '0';
        }
        else
            good = 0;
    }
    switch (x) {
        case 3:
            ret = ( ((el[0] << 24) & 0xff000000) |
                    ((el[1] << 16) & 0x00ff0000) |
                    ((el[2] << 8 ) & 0x0000ff00) |
                     (el[3] & 0x000000ff) );
            break;
        case 2:
            ret = ( ((el[0] << 24) & 0xff000000) |
                    ((el[1] << 16) & 0x00ff0000) |
                     (el[2] & 0x0000ffff) );
            break;
        case 1:
            ret = (((el[0] << 24) & 0xff000000) |
                    (el[1] & 0x00ffffff) );
            break;
        case 0:
            ret = el[0];
            break;
    }
    *retval = ret;
    return good;
}

