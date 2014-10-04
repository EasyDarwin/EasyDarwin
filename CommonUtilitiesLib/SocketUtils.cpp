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
    File:       SocketUtils.cpp

    Contains:   Implements utility functions defined in SocketUtils.h
                    

    
    
*/

#include <string.h>

#ifndef __Win32__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>

#if __FreeBSD__
#include <ifaddrs.h>
#endif
#include <unistd.h>
#include <sys/utsname.h>

#if __solaris__
#include <sys/sockio.h>
#endif
#endif

#include "SocketUtils.h"

#ifdef SIOCGIFNUM
#define USE_SIOCGIFNUM 1
#endif

#ifdef TRUCLUSTER  /* Tru64 Cluster Alias support */
  
#include <clua/clua.h>
#include <sys/clu.h>

static clua_status_t (*clua_getaliasaddress_vector) (struct sockaddr *, int *);
static char *(*clua_error_vector) (clua_status_t);

#define clua_getaliasaddress (*clua_getaliasaddress_vector)
#define clua_error (*clua_error_vector)

struct clucall_vector clua_vectors[] = {
        { "clua_getaliasaddress", &clua_getaliasaddress_vector },
        { "clua_error", &clua_error_vector },
        { NULL,     NULL }              /* END OF LIST */
};

#endif /* TRUCLUSTER */

UInt32                          SocketUtils::sNumIPAddrs = 0;
SocketUtils::IPAddrInfo*        SocketUtils::sIPAddrInfoArray = NULL;
OSMutex SocketUtils::sMutex;

#if __FreeBSD__

//Complete rewrite for FreeBSD. 
//The non-FreeBSD version really needs to be rewritten - it's a bit of a mess...
void SocketUtils::Initialize(Bool16 lookupDNSName)
{
    struct ifaddrs* ifap;
    struct ifaddrs* currentifap;
    struct sockaddr_in* sockaddr;   
    int result = 0;
    
    result = getifaddrs(&ifap);
    
    //Count them first
    currentifap = ifap;
    while( currentifap != NULL )
    {
        sockaddr = (struct sockaddr_in*)currentifap->ifa_addr;
        if (sockaddr->sin_family == AF_INET)
            sNumIPAddrs++;
        currentifap = currentifap->ifa_next;
    }
    
    
    //allocate the IPAddrInfo array. Unfortunately we can't allocate this
    //array the proper way due to a GCC bug
    UInt8* addrInfoMem = new UInt8[sizeof(IPAddrInfo) * sNumIPAddrs];
    ::memset(addrInfoMem, 0, sizeof(IPAddrInfo) * sNumIPAddrs);
    sIPAddrInfoArray = (IPAddrInfo*)addrInfoMem;
        
    int addrArrayIndex = 0;
    currentifap = ifap;
    while( currentifap != NULL )
    {
        sockaddr = (struct sockaddr_in*)currentifap->ifa_addr;
    
        if (sockaddr->sin_family == AF_INET)
        {
            char* theAddrStr = ::inet_ntoa(sockaddr->sin_addr);

            //store the IP addr
            sIPAddrInfoArray[addrArrayIndex].fIPAddr = ntohl(sockaddr->sin_addr.s_addr);

            //store the IP addr as a string
            sIPAddrInfoArray[addrArrayIndex].fIPAddrStr.Len = ::strlen(theAddrStr);
            sIPAddrInfoArray[addrArrayIndex].fIPAddrStr.Ptr = new char[sIPAddrInfoArray[addrArrayIndex].fIPAddrStr.Len + 2];
            ::strcpy(sIPAddrInfoArray[addrArrayIndex].fIPAddrStr.Ptr, theAddrStr);

            struct hostent* theDNSName = NULL;
            if (lookupDNSName) //convert this addr to a dns name, and store it
            {   theDNSName = ::gethostbyaddr((char *)&sockaddr->sin_addr, sizeof(sockaddr->sin_addr), AF_INET);
            }
            
            if (theDNSName != NULL)
            {
                sIPAddrInfoArray[addrArrayIndex].fDNSNameStr.Len = ::strlen(theDNSName->h_name);
                sIPAddrInfoArray[addrArrayIndex].fDNSNameStr.Ptr = new char[sIPAddrInfoArray[addrArrayIndex].fDNSNameStr.Len + 2];
                ::strcpy(sIPAddrInfoArray[addrArrayIndex].fDNSNameStr.Ptr, theDNSName->h_name);
            }
            else
            {
                //if we failed to look up the DNS name, just store the IP addr as a string
                sIPAddrInfoArray[addrArrayIndex].fDNSNameStr.Len = sIPAddrInfoArray[addrArrayIndex].fIPAddrStr.Len;
                sIPAddrInfoArray[addrArrayIndex].fDNSNameStr.Ptr = new char[sIPAddrInfoArray[addrArrayIndex].fDNSNameStr.Len + 2];
                ::strcpy(sIPAddrInfoArray[addrArrayIndex].fDNSNameStr.Ptr, sIPAddrInfoArray[addrArrayIndex].fIPAddrStr.Ptr);
            }

            addrArrayIndex++;
        }
        
        currentifap = currentifap->ifa_next;
    }
    
    
}

#else //__FreeBSD__

//Version for all non-FreeBSD platforms.

void SocketUtils::Initialize(Bool16 lookupDNSName)
{
#if defined(__Win32__) || defined(USE_SIOCGIFNUM)

    int tempSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (tempSocket == -1)
        return;

#ifdef __Win32__

    static const UInt32 kMaxAddrBufferSize = 2048;
    char inBuffer[kMaxAddrBufferSize];
    char outBuffer[kMaxAddrBufferSize];
    UInt32 theReturnedSize = 0;
    
    //
    // Use the WSAIoctl function call to get a list of IP addresses
    int theErr = ::WSAIoctl(    tempSocket, SIO_GET_INTERFACE_LIST, 
                                inBuffer, kMaxAddrBufferSize,
                                outBuffer, kMaxAddrBufferSize,
                                &theReturnedSize,
                                NULL,
                                NULL);
    Assert(theErr == 0);
    if (theErr != 0)
        return;
    
    Assert((theReturnedSize % sizeof(INTERFACE_INFO)) == 0);    
    LPINTERFACE_INFO addrListP = (LPINTERFACE_INFO)&outBuffer[0];
    
    sNumIPAddrs = theReturnedSize / sizeof(INTERFACE_INFO);
#else
#if defined(USE_SIOCGIFNUM)
    if (::ioctl(tempSocket, SIOCGIFNUM, (char*)&sNumIPAddrs) == -1)
    {
#ifdef MAXIFS
        sNumIPAddrs = MAXIFS;
#else
        sNumIPAddrs = 64;
#endif
    }
#else
#error
#endif
    struct ifconf ifc;
    ::memset(&ifc,0,sizeof(ifc));
    ifc.ifc_len = sNumIPAddrs * sizeof(struct ifreq);
    ifc.ifc_buf = (caddr_t)new struct ifreq[sNumIPAddrs];
    Assert(ifc.ifc_buf != NULL);

    ::memset(ifc.ifc_buf, '\0', ifc.ifc_len);
    int theErr = ::ioctl(tempSocket, SIOCGIFCONF, (char*)&ifc);
    Assert(theErr == 0);
    if (theErr != 0)
        return;
    struct ifreq* ifr = (struct ifreq*)ifc.ifc_buf;
#endif
    
    //allocate the IPAddrInfo array. Unfortunately we can't allocate this
    //array the proper way due to a GCC bug
    UInt8* addrInfoMem = new UInt8[sizeof(IPAddrInfo) * sNumIPAddrs];
    ::memset(addrInfoMem, 0, sizeof(IPAddrInfo) * sNumIPAddrs);
    sIPAddrInfoArray = (IPAddrInfo*)addrInfoMem;

    //for (UInt32 addrCount = 0; addrCount < sNumIPAddrs; addrCount++)
    UInt32 currentIndex = 0;
    for (UInt32 theIfCount = sNumIPAddrs, addrCount = 0;
         addrCount < theIfCount; addrCount++)
    {
#ifdef __Win32__
        // We *should* count the loopback interface as a valid interface.
        //if (addrListP[addrCount].iiFlags & IFF_LOOPBACK)
        //{
            // Don't count loopback addrs
        //  sNumIPAddrs--;
        //  continue;
        //}
        //if (addrListP[addrCount].iiFlags & IFF_LOOPBACK)
        //  if (lookupDNSName) // The playlist broadcaster doesn't care
        //      Assert(addrCount > 0); // If the loopback interface is interface 0, we've got problems
            
        struct sockaddr_in* theAddr = (struct sockaddr_in*)&addrListP[addrCount].iiAddress;
#elif defined(USE_SIOCGIFNUM)
        if (ifr[addrCount].ifr_addr.sa_family != AF_INET)
        {
            sNumIPAddrs--;
            continue;
        }
        struct ifreq ifrf;
        ::memset(&ifrf,0,sizeof(ifrf));
        ::strncpy(ifrf.ifr_name, ifr[addrCount].ifr_name, sizeof(ifrf.ifr_name));
        theErr = ::ioctl(tempSocket, SIOCGIFFLAGS, (char *) &ifrf);
        Assert(theErr != -1);

#ifndef __solaris__
        /* Skip things which aren't interesting */
        if ((ifrf.ifr_flags & IFF_UP) == 0 ||
            (ifrf.ifr_flags & (IFF_BROADCAST | IFF_POINTOPOINT)) == 0)
        {
            sNumIPAddrs--;
            continue;
        }
        if (ifrf.ifr_flags & IFF_LOOPBACK)
        {
            Assert(addrCount > 0); // If the loopback interface is interface 0, we've got problems
        }
#endif
        
        struct sockaddr_in* theAddr = (struct sockaddr_in*)&ifr[addrCount].ifr_addr;    
    #if 0
        puts(ifr[addrCount].ifr_name);
    #endif
#else
#error
#endif

        char* theAddrStr = ::inet_ntoa(theAddr->sin_addr);

        //store the IP addr
        sIPAddrInfoArray[currentIndex].fIPAddr = ntohl(theAddr->sin_addr.s_addr);
        
        //store the IP addr as a string
        sIPAddrInfoArray[currentIndex].fIPAddrStr.Len = ::strlen(theAddrStr);
        sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fIPAddrStr.Len + 2];
        ::strcpy(sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr, theAddrStr);

        
        struct hostent* theDNSName = NULL;
        if (lookupDNSName) //convert this addr to a dns name, and store it
        {   theDNSName = ::gethostbyaddr((char *)&theAddr->sin_addr, sizeof(theAddr->sin_addr), AF_INET);
        }
        
        if (theDNSName != NULL)
        {
            sIPAddrInfoArray[currentIndex].fDNSNameStr.Len = ::strlen(theDNSName->h_name);
            sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fDNSNameStr.Len + 2];
            ::strcpy(sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr, theDNSName->h_name);
        }
        else
        {
            //if we failed to look up the DNS name, just store the IP addr as a string
            sIPAddrInfoArray[currentIndex].fDNSNameStr.Len = sIPAddrInfoArray[currentIndex].fIPAddrStr.Len;
            sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fDNSNameStr.Len + 2];
            ::strcpy(sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr, sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr);
        }
        //move onto the next array index
        currentIndex++;
        
    }
#ifdef __Win32__
    ::closesocket(tempSocket);
#elif defined(USE_SIOCGIFNUM)
    delete[] ifc.ifc_buf;
    ::close(tempSocket);
#else
#error
#endif
    
#else // !__Win32__

    //Most of this code is similar to the SIOCGIFCONF code presented in Stevens,
    //Unix Network Programming, section 16.6
    
    //Use the SIOCGIFCONF ioctl call to iterate through the network interfaces
    static const UInt32 kMaxAddrBufferSize = 2048;
    
    struct ifconf ifc;
    ::memset(&ifc,0,sizeof(ifc));
    struct ifreq* ifr;
    char buffer[kMaxAddrBufferSize];
    
    int tempSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (tempSocket == -1)
        return;
        
    ifc.ifc_len = kMaxAddrBufferSize;
    ifc.ifc_buf = buffer;

#if __linux__ || __linuxppc__ || __solaris__ || __MacOSX__ || __sgi__ || __osf__
    int err = ::ioctl(tempSocket, SIOCGIFCONF, (char*)&ifc);
#elif __FreeBSD__
    int err = ::ioctl(tempSocket, OSIOCGIFCONF, (char*)&ifc);
#else
    #error
#endif
    if (err == -1)
        return;

#if __FreeBSD__
    int netdev1, netdev2;
    struct ifreq *netdevifr;
    netdevifr = ifc.ifc_req;
    netdev1 = ifc.ifc_len / sizeof(struct ifreq);
    for (netdev2=netdev1-1; netdev2>=0; netdev2--)
        {
        if (ioctl(tempSocket, SIOCGIFADDR, &netdevifr[netdev2]) != 0)
            continue;
        }
#endif
    
    ::close(tempSocket);
    tempSocket = -1;

    //walk through the list of IP addrs twice. Once to find out how many,
    //the second time to actually grab their information
    char* ifReqIter = NULL;
    sNumIPAddrs = 0;
    
    for (ifReqIter = buffer; ifReqIter < (buffer + ifc.ifc_len);)
    {
        ifr = (struct ifreq*)ifReqIter;
        if (!SocketUtils::IncrementIfReqIter(&ifReqIter, ifr))
            return;
        
        // Some platforms have lo as the first interface, so we have code to
        // work around this problem below
        //if (::strncmp(ifr->ifr_name, "lo", 2) == 0)
        //  Assert(sNumIPAddrs > 0); // If the loopback interface is interface 0, we've got problems
    
        //Only count interfaces in the AF_INET family.
        if (ifr->ifr_addr.sa_family == AF_INET)
            sNumIPAddrs++;
    }

#ifdef TRUCLUSTER
    
    int clusterAliases = 0;

    if (clu_is_member())
    {
        /* loading the vector table */
      if (clua_getaliasaddress_vector == NULL)
      {
        clucall_stat    clustat;
        struct sockaddr_in  sin;

        clustat = clucall_load("libclua.so", clua_vectors);
        int context = 0;
        clua_status_t      addr_err;

        if (clua_getaliasaddress_vector != NULL)
          while ( (addr_err = clua_getaliasaddress
              ((struct sockaddr*)&sin, &context)) == CLUA_SUCCESS )
          {
        sNumIPAddrs++;
        clusterAliases++;
          }
      }
      
    }

#endif // TRUCLUSTER
    
    //allocate the IPAddrInfo array. Unfortunately we can't allocate this
    //array the proper way due to a GCC bug
    UInt8* addrInfoMem = new UInt8[sizeof(IPAddrInfo) * sNumIPAddrs];
    ::memset(addrInfoMem, 0, sizeof(IPAddrInfo) * sNumIPAddrs);
    sIPAddrInfoArray = (IPAddrInfo*)addrInfoMem;
    
    //Now extract all the necessary information about each interface
    //and put it into the array
    UInt32 currentIndex = 0;

#ifdef TRUCLUSTER
	// Do these cluster aliases first so they'll be first in the list
	if (clusterAliases > 0)
	{
		int context = 0;
		struct sockaddr_in sin;
		clua_status_t      addr_err;

		while ( (addr_err = clua_getaliasaddress ((struct sockaddr*)&sin, &context)) == CLUA_SUCCESS )
		{
			char* theAddrStr = ::inet_ntoa(sin.sin_addr);
 
			//store the IP addr
			sIPAddrInfoArray[currentIndex].fIPAddr = ntohl(sin.sin_addr.s_addr);
 	    
			//store the IP addr as a string
			sIPAddrInfoArray[currentIndex].fIPAddrStr.Len = ::strlen(theAddrStr);
			sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fIPAddrStr.Len + 2];
			::strcpy(sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr, theAddrStr);
 
			//convert this addr to a dns name, and store it
			struct hostent* theDNSName = ::gethostbyaddr((char *)&sin.sin_addr,
											sizeof(sin.sin_addr), AF_INET);
			if (theDNSName != NULL)
			{
				sIPAddrInfoArray[currentIndex].fDNSNameStr.Len = ::strlen(theDNSName->h_name);
				sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fDNSNameStr.Len + 2];
				::strcpy(sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr, theDNSName->h_name);
			}
			else
			{
				//if we failed to look up the DNS name, just store the IP addr as a string
				sIPAddrInfoArray[currentIndex].fDNSNameStr.Len = sIPAddrInfoArray[currentIndex].fIPAddrStr.Len;
				sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fDNSNameStr.Len + 2];
				::strcpy(sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr, sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr);
			}
 
			currentIndex++;
		}
	}
#endif // TRUCLUSTER	
  	
    for (ifReqIter = buffer; ifReqIter < (buffer + ifc.ifc_len);)
    {
        ifr = (struct ifreq*)ifReqIter;
        if (!SocketUtils::IncrementIfReqIter(&ifReqIter, ifr))
        {
            Assert(0);//we should have already detected this error
            return;
        }
        
        //Only count interfaces in the AF_INET family
        if (ifr->ifr_addr.sa_family == AF_INET)
        {
            struct sockaddr_in* addrPtr = (struct sockaddr_in*)&ifr->ifr_addr;  
            char* theAddrStr = ::inet_ntoa(addrPtr->sin_addr);

            //store the IP addr
            sIPAddrInfoArray[currentIndex].fIPAddr = ntohl(addrPtr->sin_addr.s_addr);
            
            //store the IP addr as a string
            sIPAddrInfoArray[currentIndex].fIPAddrStr.Len = ::strlen(theAddrStr);
            sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fIPAddrStr.Len + 2];
            ::strcpy(sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr, theAddrStr);

            struct hostent* theDNSName = NULL;
            if (lookupDNSName) //convert this addr to a dns name, and store it
            {   theDNSName = ::gethostbyaddr((char *)&addrPtr->sin_addr, sizeof(addrPtr->sin_addr), AF_INET);
            }
            
            if (theDNSName != NULL)
            {
                sIPAddrInfoArray[currentIndex].fDNSNameStr.Len = ::strlen(theDNSName->h_name);
                sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fDNSNameStr.Len + 2];
                ::strcpy(sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr, theDNSName->h_name);
            }
            else
            {
                //if we failed to look up the DNS name, just store the IP addr as a string
                sIPAddrInfoArray[currentIndex].fDNSNameStr.Len = sIPAddrInfoArray[currentIndex].fIPAddrStr.Len;
                sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fDNSNameStr.Len + 2];
                ::strcpy(sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr, sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr);
            }
            
            //move onto the next array index
            currentIndex++;
        }
    }
    
    Assert(currentIndex == sNumIPAddrs);
#endif

    //
    // If LocalHost is the first element in the array, switch it to be the second.
    // The first element is supposed to be the "default" interface on the machine,
    // which should really always be en0.
    if ((sNumIPAddrs > 1) && (::strcmp(sIPAddrInfoArray[0].fIPAddrStr.Ptr, "127.0.0.1") == 0))
    {
        UInt32 tempIP = sIPAddrInfoArray[1].fIPAddr;
        sIPAddrInfoArray[1].fIPAddr = sIPAddrInfoArray[0].fIPAddr;
        sIPAddrInfoArray[0].fIPAddr = tempIP;
        StrPtrLen tempIPStr(sIPAddrInfoArray[1].fIPAddrStr);
        sIPAddrInfoArray[1].fIPAddrStr = sIPAddrInfoArray[0].fIPAddrStr;
        sIPAddrInfoArray[0].fIPAddrStr = tempIPStr;
        StrPtrLen tempDNSStr(sIPAddrInfoArray[1].fDNSNameStr);
        sIPAddrInfoArray[1].fDNSNameStr = sIPAddrInfoArray[0].fDNSNameStr;
        sIPAddrInfoArray[0].fDNSNameStr = tempDNSStr;
    }
}
#endif  //__FreeBSD__



#ifndef __Win32__
Bool16 SocketUtils::IncrementIfReqIter(char** inIfReqIter, ifreq* ifr)
{
    //returns true if successful, false otherwise

#if __MacOSX__
    *inIfReqIter += sizeof(ifr->ifr_name) + ifr->ifr_addr.sa_len;

    //if the length of the addr is 0, use the family to determine
    //what the addr size is
    if (ifr->ifr_addr.sa_len == 0)
    {
        switch (ifr->ifr_addr.sa_family)
        {
            case AF_INET:
                *inIfReqIter += sizeof(struct sockaddr_in);
                break;
            default:
                *inIfReqIter += sizeof(struct sockaddr);
//              Assert(0);
//              sNumIPAddrs = 0;
//              return false;
        }
    }
#else
    *inIfReqIter += sizeof(*ifr);
#endif
    return true;
}
#endif

Bool16 SocketUtils::IsMulticastIPAddr(UInt32 inAddress)
{
    return ((inAddress>>8) & 0x00f00000) == 0x00e00000; //  multicast addresses == "class D" == 0xExxxxxxx == 1,1,1,0,<28 bits>
}

Bool16 SocketUtils::IsLocalIPAddr(UInt32 inAddress)
{
    for (UInt32 x = 0; x < sNumIPAddrs; x++)
        if (sIPAddrInfoArray[x].fIPAddr == inAddress)
            return true;
    return false;
}

void SocketUtils::ConvertAddrToString(const struct in_addr& theAddr, StrPtrLen* ioStr)
{
    //re-entrant version of code below
    //inet_ntop(AF_INET, &theAddr, ioStr->Ptr, ioStr->Len);
    //ioStr->Len = ::strlen(ioStr->Ptr);
    
    sMutex.Lock();
    char* addr = inet_ntoa(theAddr);
    strcpy(ioStr->Ptr, addr);
    ioStr->Len = ::strlen(ioStr->Ptr);
    sMutex.Unlock();
}

UInt32 SocketUtils::ConvertStringToAddr(const char* inAddrStr)
{
    if (inAddrStr == NULL)
        return 0;
        
    return ntohl(::inet_addr(inAddrStr));
}

