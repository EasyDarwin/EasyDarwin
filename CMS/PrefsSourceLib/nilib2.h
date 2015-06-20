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
	Copyleft (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/


#include <stdio.h>
#include <netinfo/ni.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <strings.h>
#include <pwd.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif


enum ni_parse_status {
    NI_PARSE_OK = 0,
    NI_PARSE_BADFORMAT = 1,
    NI_PARSE_NOHOST = 2,
    NI_PARSE_NOTAG = 3,
    NI_PARSE_BADADDR = 4,
    NI_PARSE_HOSTNOTFOUND = 5
};

/* first arg is input, second two args are output address and tag */
enum ni_parse_status
ni_parse_server_tag(char *, struct sockaddr_in *, char **);

/* returns an error string for the input status */
const char *ni_parse_error_string(enum ni_parse_status);

/* first arg is the name of the calling program (argv[0]) */
/* second arg is the domain name (input) */
/* third arg is output domain handle */
/* fourth arg is TRUE if open by tag */
/* fifth arg is read & write timeout in seconds */
/* sixth arg is user name (NULL for no user) */
/* seventh arg is password (NULL for no password) */
/*
 * returns 0            on success
 * returns 0<x<10000 (ni_status) for NetInfo errors
 * returns x=10000   (NI_FAILED+1) for ni_connect() errors
 * returns x>10000   (NI_FAILED+1+ni_parse_status) for error in NetInfo tag
 */
int do_open(char *, char *, void **, bool, int, char *, char *);

ni_status ni2_create(void *domain, char *pathname);
ni_status ni2_createprop(void *domain, char *pathname, const ni_name key, ni_namelist values);
ni_status ni2_createdirprop(void *domain, ni_id *dir, const ni_name key, ni_namelist values);
ni_status ni2_appendprop(void *domain, char *pathname, const ni_name key, ni_namelist values);
ni_status ni2_appenddirprop(void *domain, ni_id *dir, const ni_name key, ni_namelist values);
ni_status ni2_mergeprop(void *domain, char *pathname, const ni_name key, ni_namelist values);
ni_status ni2_mergedirprop(void *domain, ni_id *dir, const ni_name key, ni_namelist values);
ni_status ni2_insertval(void *domain, char *pathname, const ni_name key, const ni_name value, ni_index where);
ni_status ni2_insertdirval(void *domain, ni_id *dir, const ni_name key, const ni_name value, ni_index whereval);
ni_status ni2_destroy(void *domain, char *pathname);
ni_status ni2_destroydir(void *domain, ni_id *dir, ni_id *parent);
ni_status ni2_destroyprop(void *domain, char *pathname, ni_namelist keys);
ni_status ni2_destroydirprop(void *domain, ni_id *dir, ni_namelist keys);
ni_status ni2_destroyval(void *domain, char *pathname, const ni_name key, ni_namelist values);
ni_status ni2_destroydirval(void *domain, ni_id *dir, const ni_name key, ni_namelist values);
ni_status ni2_renameprop(void *domain, char *pathname, const ni_name oldname, const ni_name newname);
ni_status ni2_renamedirprop(void *domain, ni_id *dir, const ni_name oldname, const ni_name newname);
ni_status ni2_createpath(void *domain, ni_id *dir, char *pathname);
ni_status ni2_createchild(void *domain, ni_id *dir, const ni_name dirname);
void nipl_createprop(ni_proplist *l, const ni_name n);
void nipl_appendprop(ni_proplist *l, const ni_name n, const ni_name v);
void nipl_mergeprop(ni_proplist *l, const ni_name n, const ni_name v);
ni_status ni2_rparent(void *domain, struct sockaddr_in *addr, char **tag);
ni_status ni2_pathsearch(void *domain, ni_id *dir, char *pathname);
ni_status ni2_statprop(void *domain, char *pathname, const ni_name key, ni_index *where);
ni_status ni2_statpropdir(void *domain, ni_id *dir, const ni_name key, ni_index *where);
ni_status ni2_statval(void *domain, char *pathname, const ni_name key, const ni_name value, ni_index *where);
ni_status ni2_statvaldir(void *domain, ni_id *dir, const ni_name key, const ni_name value, ni_index *where);
ni_status ni2_reapprop(void *domain, char *pathname, const ni_name key);
ni_status ni2_reappropdir(void *domain, ni_id *dir, const ni_name key);
ni_status ni2_reapdir(void *domain, char *pathname);
ni_status ni2_copy(void *srcdomain, char *srcpath, void*dstdomain, bool recursive);
ni_status ni2_copydir(void *srcdomain, ni_id *srcdir, void*dstdomain, ni_id *dstdir, bool recursive);
ni_status ni2_copydirtoparentdir(void *srcdomain, ni_id *srcdir, void*dstdomain, ni_id *dstdir, bool recursive);
ni_status ni2_lookupprop(void *domain, char *pathname, const ni_name key, ni_namelist *values);
ni_index ni_namelist_insert_sorted(ni_namelist *values, const ni_name newvalue);
;
#ifdef __cplusplus
}
#endif
