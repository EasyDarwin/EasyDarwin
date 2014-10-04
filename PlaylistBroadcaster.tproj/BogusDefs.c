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
#include "BogusDefs.h"
#include "mymutex.h"

ssize_t recvfrom (int, void *, size_t, int, struct sockaddr *, int *)
{
    return 0;
}

ssize_t sendto (int, const void *, size_t, int, const struct sockaddr *, int)
{
    return 0;
}

int daemon (int, int)
{
    return 0;
}

int inet_aton (const char *, struct in_addr *)
{
    return 0;
}

int bind(int, struct sockaddr *, int)
{
    return 0;
}

char * inet_ntoa(struct in_addr)
{
    return 0;
}

int ioctl(int, UInt32, char*)
{
    return 0;
}

int socket(int, int, int)
{
    return 0;
}


void mymutex_unlock(void* x)
{
    x =x;
}

void mymutex_free(void* x)
{
    x =x;

}

void mymutex_lock(void* x)
{
    x =x;
}

SInt64 timestamp = 0;


int ftruncate(int fd, int length)
{
    return 0;
}

mymutex_t mymutex_alloc()
{   
    return 0;
}

char *optarg = NULL;