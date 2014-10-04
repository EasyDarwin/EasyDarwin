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
#include "GetLocalIPAddressString.h"


#ifdef __MACOS__
    #include "BogusDefs.h"
#else
    #include <string.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <net/if.h>
    

#endif

#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <unistd.h>
#include <string.h>

#if __solaris__ || __sgi__
    #include <sys/sockio.h>
    #include <stropts.h>
#endif


short GetLocalIPAddressString(char *returnStr, short maxSize)
{
    short result = 0;
    int err = -1;
        
    char defaultAddress[] = "255.255.255.255";
    char* addr = defaultAddress;
    do 
    {
    
        //Most of this code is similar to the SIOCGIFCONF code presented in Stevens,
        //Unix Network Programming, section 16.6
        
        //Use the SIOCGIFCONF ioctl call to iterate through the network interfaces
        static const UInt32 kMaxAddrBufferSize = 2048;
        
        char* ifReqIter = NULL;
        struct ifconf ifc;
        struct ifreq* ifr;
        char buffer[kMaxAddrBufferSize];
        
        int tempSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (tempSocket == -1) break;
            
        ifc.ifc_len = kMaxAddrBufferSize;
        ifc.ifc_buf = buffer;
        
#if (__linux__ || __MacOSX__ || __MACOS__ || __linuxppc__ || __solaris__ || __sgi__)
        err = ioctl(tempSocket, SIOCGIFCONF, (char*)&ifc);
#elif __FreeBSD__
        err = ioctl(tempSocket, OSIOCGIFCONF, (char*)&ifc);
#else
        #error
#endif
        if (err == -1) break;
        

#if __FreeBSD__
        {
            int netdev1, netdev2;
            struct ifreq *netdevifr;
            netdevifr = ifc.ifc_req;
            netdev1 = ifc.ifc_len / sizeof(struct ifreq);
            for (netdev2=netdev1-1; netdev2>=0; netdev2--)
                {
                if (ioctl(tempSocket, SIOCGIFADDR, &netdevifr[netdev2]) != 0)
                    continue;
                }
        }
#endif

        
        close(tempSocket);
        tempSocket = -1;

//      int numIPAddrs = 0;
        
        for (ifReqIter = buffer; ifReqIter < (buffer + ifc.ifc_len);)
        {
            ifr = (struct ifreq*)ifReqIter;
#if __MacOSX__ 
            ifReqIter += sizeof(ifr->ifr_name) + ifr->ifr_addr.sa_len;
            if (ifr->ifr_addr.sa_len == 0)
            {
                switch (ifr->ifr_addr.sa_family)
                {
                    case AF_INET:
                        ifReqIter += sizeof(struct sockaddr_in);
                        break;
                    default:
                        ifReqIter += sizeof(struct sockaddr);
                }
            }
#else
            ifReqIter += sizeof(ifr->ifr_name) + 0;
            switch (ifr->ifr_addr.sa_family)
            {
                case AF_INET:
                    ifReqIter += sizeof(struct sockaddr_in);
                    break;
                default:
                    ifReqIter += sizeof(struct sockaddr);
            }
#endif
                
            //Only count interfaces in the AF_INET family.
            //And don't count localhost, loopback interfaces
            if ((ifr->ifr_addr.sa_family == AF_INET) && (strncmp(ifr->ifr_name, "lo", 2) != 0))
            {
                struct sockaddr_in* addrPtr = (struct sockaddr_in*)&ifr->ifr_addr;  
                addr = inet_ntoa(addrPtr->sin_addr);
                //qtss_printf("found local address: %s\n", addr);
                err = 0;
                break;
            }
        }
    
    } while (0);
        

    result = strlen(addr);
    
    if (maxSize < result)
    {   err = -1;
    }
    else
        strcpy(returnStr, addr);

    return err;
}
