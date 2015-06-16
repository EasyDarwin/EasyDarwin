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
 * nilib2 ± more NetInfo library routines.
 *
 */

#include "nilib2.h"
#include <stdlib.h>
#include "SafeStdLib.h"
#include <rpc/rpc.h>

enum ni_parse_status
ni_parse_server_tag(char *str, struct sockaddr_in *server, char **t)
{
    /* utility to parse a server/tag string */

    int len, i;
    char *host, *tag, *slash;
    struct hostent *hent;

    len = strlen(str);

    /* find the "/" character */
    slash = index(str, '/');

    /* check to see if the "/" is missing */
    if (slash == NULL) return NI_PARSE_BADFORMAT;

    /* find the location of the '/' */
    i = slash - str;

    /* check if host string is empty */
    if (i == 0) return NI_PARSE_NOHOST;

    /* check if tag string is empty */
    if (i == (len - 1)) return NI_PARSE_NOTAG;

    /* allocate some space for the host and tag */
    host = (char *)malloc(i + 1);
    *t = (char *)malloc(len - i);
    tag = *t;

    /* copy out the host */
    strncpy(host, str, i);
    host[i] = '\0';

    /* copy out the tag */
    strcpy(tag, slash + 1);

    /* try interpreting the host portion as an address */
    server->sin_addr.s_addr = inet_addr(host);

    if (server->sin_addr.s_addr == -1)
    {
        /* This isn't a valid address.  Is it a known hostname? */
        hent = gethostbyname(host);
        if (hent != NULL)
        {
            /* found a host with that name */
            bcopy(hent->h_addr, &server->sin_addr, hent->h_length);
        }
        else
        {
            qtss_fprintf(stderr, "Can't find address for %s\n", host);
            free(host);
            free(tag);
            return NI_PARSE_HOSTNOTFOUND;
        }
   }

    free(host);
    return NI_PARSE_OK;
}

const char *
ni_parse_error_string(enum ni_parse_status status)
{
    switch (status)
    {
        case NI_PARSE_OK: return("Operation succeeded");
        case NI_PARSE_BADFORMAT: return("Bad format");
        case NI_PARSE_NOHOST: return("No host");
        case NI_PARSE_NOTAG: return("No tag");
        case NI_PARSE_BADADDR: return("Bad address");
        case NI_PARSE_HOSTNOTFOUND: return("Host not found");
    }
    return NULL;
}

int
do_open(char *tool, char *name, void **domain, bool bytag, int timeout, char *user, char *passwd)
{
    /* do an ni_open or an ni_connect, as appropriate */

    char *tag;
    enum ni_parse_status pstatus;
    ni_status status;
    struct sockaddr_in server;
    ni_id rootdir;

    if (bytag) {
        /* connect by tag */
        /* call a function to parse the input arg */
        pstatus = ni_parse_server_tag(name, &server, &tag);
        if (pstatus != NI_PARSE_OK)
        {
            qtss_fprintf(stderr, "%s: incorrect format for domain %s (%s)\n",
                tool, name, ni_parse_error_string(pstatus));
            qtss_fprintf(stderr, "usage: -t <host>/<tag>\n");
            qtss_fprintf(stderr, "<host> can be a host name or IP address\n");
                        return NI_FAILED + 1 + pstatus;
        }

        /* connect to the specified server */
        *domain = ni_connect(&server, tag);
        if (*domain == NULL) {
            qtss_fprintf(stderr, "%s: can't connect to server %s\n", tool, name);
            free(tag);
                        return NI_FAILED + 1;
        }
    }

    else {
        /* open domain */
        status = ni_open(NULL, name, domain);
        if (status != NI_OK) {
            qtss_fprintf(stderr, "%s: can't connect to server for domain %s\n",
                tool, name);
                        return status;
        }
    }

    /* abort on errors */
    ni_setabort(*domain, 1);
    
    /* set timeouts */
    ni_setreadtimeout(*domain, timeout);
    ni_setwritetimeout(*domain, timeout);

    /* authentication */
    if (user != NULL) {
        ni_setuser(*domain, user);
        if (passwd != NULL) ni_setpassword(*domain, passwd);
    }

    /* get the root directory to see if the connection is alive */
    status = ni_root(*domain, &rootdir);
    if (status != NI_OK) {
        if (bytag)
            qtss_fprintf(stderr, "%s: can't connect to server %s: %s\n",
                tool, name, ni_error(status));
        else
            qtss_fprintf(stderr, "%s: can't connect to server for domain %s: %s\n",
                tool, name, ni_error(status));
                return status;
    }

    return 0;
}

ni_status ni2_create(void *domain, char *pathname)
{
    /* make a directory with the given pathname */
    /* do nothing if the directory already exists */

    ni_status ret;
    ni_id dir;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* see if it already exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret == NI_OK) return NI_OK;

    /* doesn't exist: create it */
    ret = ni_root(domain, &dir);
    if (ret != NI_OK) return ret;

    if (pathname[0] == '/') ret = ni2_createpath(domain, &dir, pathname+1);
    else ret = ni2_createpath(domain, &dir, pathname);

    return ret; 
}

ni_status ni2_createprop(void *domain, char *pathname, const ni_name key, ni_namelist values)
{
    /* create a new property with a given key and list of values */
    /* replaces an existing property if it already exists */

    ni_status ret;
    ni_id dir;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* see if the directory already exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    return ni2_createdirprop(domain, &dir, key, values);
}

ni_status ni2_createdirprop(void *domain, ni_id *dir, const ni_name key, ni_namelist values)
{
    /* createprop given a directory rather than a pathname */

    ni_status ret;
    ni_property p;
    ni_namelist nl;
    ni_index where;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* fetch list of property keys from directory */
    NI_INIT(&nl);
    ret = ni_listprops(domain, dir, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* check for existing property with this key */
    where = ni_namelist_match(nl, key);
    ni_namelist_free(&nl);

    /* if property doesn't exist, create it */
    if (where == NI_INDEX_NULL) {
        NI_INIT(&p);
        p.nip_name = ni_name_dup(key);
        p.nip_val = ni_namelist_dup(values);
        ret = ni_createprop(domain, dir, p, NI_INDEX_NULL);
        ni_prop_free(&p);
        return ret;
    }

    /* property exists: replace the existing values */
    ret = ni_writeprop(domain, dir, where, values);
    return ret;
}

ni_status ni2_appendprop(void *domain, char *pathname, const ni_name key, ni_namelist values)
{
    /* append a list of values to a property */
    /* a new property is created if it doesn't exist */

    ni_status ret;
    ni_id dir;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* see if the directory already exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    return ni2_appenddirprop(domain, &dir, key, values);
}

ni_status ni2_appenddirprop(void *domain, ni_id *dir, const ni_name key, ni_namelist values)
{
    /* appendprop given a directory rather than a pathname */

    ni_status ret;
    ni_property p;
    ni_namelist nl;
    ni_index where;
    int i;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* fetch list of property keys from directory */
    NI_INIT(&nl);
    ret = ni_listprops(domain, dir, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* check for existing property with this key */
    where = ni_namelist_match(nl, key);
    ni_namelist_free(&nl);

    /* if property doesn't exist, create it */
    if (where == NI_INDEX_NULL) {
        NI_INIT(&p);
        p.nip_name = ni_name_dup(key);
        p.nip_val = ni_namelist_dup(values);
        ret = ni_createprop(domain, dir, p, NI_INDEX_NULL);
        ni_prop_free(&p);
        return ret;
    }


    /* property exists: replace the existing values */
    /* fetch existing namelist for this property */
    NI_INIT(&nl);
    ret = ni_readprop(domain, dir, where, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* append new values */
    for (i = 0; i < values.ni_namelist_len; i++) {
        ni_namelist_insert(&nl, values.ni_namelist_val[i], NI_INDEX_NULL);
    }

    /* write the new list back */
    ret = ni_writeprop(domain, dir, where, nl);

    ni_namelist_free(&nl);
    return ret;
}

ni_status ni2_insertval(void *domain, char *pathname, const ni_name key, const ni_name value, ni_index where)
{
    /* insert a new value into a property */
    /* the property is created if it doesn't exist */

    ni_status ret;
    ni_id dir;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* see if the directory exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    return ni2_insertdirval(domain, &dir, key, value, where);
}

ni_status ni2_insertdirval(void *domain, ni_id *dir, const ni_name key, const ni_name value, ni_index whereval)
{
    /* insertval given a directory rather than a pathname */

    ni_status ret;
    ni_property p;
    ni_namelist nl;
    ni_index where;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* fetch list of property keys from directory */
    NI_INIT(&nl);
    ret = ni_listprops(domain, dir, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* check for existing property with this key */
    where = ni_namelist_match(nl, key);
    ni_namelist_free(&nl);

    /* if property doesn't exist, create it */
    if (where == NI_INDEX_NULL) {
        NI_INIT(&nl);
        ni_namelist_insert(&nl, value, NI_INDEX_NULL);
        NI_INIT(&p);
        p.nip_name = ni_name_dup(key);
        p.nip_val = ni_namelist_dup(nl);
        ret = ni_createprop(domain, dir, p, NI_INDEX_NULL);
        ni_namelist_free(&nl);
        ni_prop_free(&p);
        return ret;
    }

    /* property exists: replace the existing values */
    /* fetch existing namelist for this property */
    NI_INIT(&nl);
    ret = ni_readprop(domain, dir, where, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* insert new value */
    ni_namelist_insert(&nl, value, whereval);

    /* write the new list back */
    ret = ni_writeprop(domain, dir, where, nl);
    ni_namelist_free(&nl);
    return ret;
}

ni_status ni2_mergeprop(void *domain, char *pathname, const ni_name key, ni_namelist values)
{
    /* merge a list of values into a property (to prevent duplicates) */
    /* creates the property if it doesn't already exist */

    ni_status ret;
    ni_id dir;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* see if the directory already exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    return ni2_mergedirprop(domain, &dir, key, values);
}

ni_status ni2_mergedirprop(void *domain, ni_id *dir, const ni_name key, ni_namelist values)
{
    /* mergeprop given a directory rather than a pathname */

    ni_status ret;
    ni_property p;
    ni_namelist nl;
    ni_index where, whereval;
    int i;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* fetch list of property keys from directory */
    NI_INIT(&nl);
    ret = ni_listprops(domain, dir, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* check for existing property with this key */
    where = ni_namelist_match(nl, key);
    ni_namelist_free(&nl);

    /* if property doesn't exist, create it */
    if (where == NI_INDEX_NULL) {
        NI_INIT(&p);
        p.nip_name = ni_name_dup(key);
        p.nip_val = ni_namelist_dup(values);
        ret = ni_createprop(domain, dir, p, NI_INDEX_NULL);
        ni_prop_free(&p);
        return ret;
    }


    /* property exists: replace the existing values */
    /* fetch existing namelist for this property */
    NI_INIT(&nl);
    ret = ni_readprop(domain, dir, where, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* merge new values */
    for (i = 0; i < values.ni_namelist_len; i++) {
        whereval = ni_namelist_match(nl, values.ni_namelist_val[i]);
        if (whereval == NI_INDEX_NULL) {
            ni_namelist_insert(&nl, values.ni_namelist_val[i], NI_INDEX_NULL);
        }
    }

    /* write the new list back */
    ret = ni_writeprop(domain, dir, where, nl);
    ni_namelist_free(&nl);
    return ret;
}

ni_status ni2_destroy(void *domain, char *pathname)
{
    /* destroy a directory */
    /* this version recursively destroys all subdirectories as well */

    ni_status ret;
    ni_id dir, parent;
    ni_index pi;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* see if the directory exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    /* get the parent directory index (nii_object) */
    ret = ni_parent(domain, &dir, &pi);
    if (ret != NI_OK) return ret;

    /* get the parent directory id */
    parent.nii_object = pi;
    ret = ni_self(domain, &parent);
    if (ret != NI_OK) return ret;

    return ni2_destroydir(domain, &dir, &parent);
}

ni_status ni2_destroydir(void *domain, ni_id *dir, ni_id *parent)
{
    /* destroy a directory and all it's subdirectories */
    /* this is the recursive workhorse */

    ni_status ret;
    int i;
    ni_idlist children;
    ni_id child;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* get a list of all my children */
    NI_INIT(&children);
    ret = ni_children(domain, dir, &children);
    if (ret != NI_OK) return ret;

    /* destroy each child */
    for (i = 0; i < children.ni_idlist_len; i++) {
        child.nii_object = children.ni_idlist_val[i];
        ret = ni_self(domain, &child);
        if (ret != NI_OK) return ret;
        ret = ni2_destroydir(domain, &child, dir);
        if (ret != NI_OK) return ret;
    }

    /* free list of child ids */
    ni_idlist_free(&children);

    /* destroy myself */
    return ni_destroy(domain, parent, *dir);
}

ni_status ni2_destroyprop(void *domain, char *pathname, ni_namelist keys)
{
    /* destroy a property */
    /* destroys all properties with the same key */

    ni_status ret;
    ni_id dir;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* see if the directory exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    return ni2_destroydirprop(domain, &dir, keys);
}

ni_status ni2_destroydirprop(void *domain, ni_id *dir, ni_namelist keys)
{
    /* destroyprop given a directory rather than a pathname */

    ni_status ret;
    ni_index where;
    ni_namelist nl;
    int i;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* fetch list of property keys from directory */
    NI_INIT(&nl);
    ret = ni_listprops(domain, dir, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* destroy all occurrences of each key */
    for (i = 0; i < keys.ni_namelist_len; i++) {

        where = ni_namelist_match(nl, keys.ni_namelist_val[i]);

        /* keep looking for all occurrences */
        while (where != NI_INDEX_NULL) {
            ret = ni_destroyprop(domain, dir, where);
            if (ret != NI_OK) {
                ni_namelist_free(&nl);
                return ret;
            }

            /* update the namelist */  
            ni_namelist_delete(&nl, where);
            where = ni_namelist_match(nl, keys.ni_namelist_val[i]);
        }
    }
    ni_namelist_free(&nl);
    return NI_OK;
}

ni_status ni2_destroyval(void *domain, char *pathname, const ni_name key, ni_namelist values)
{
    /* destroy all occurances of a value in a property */

    ni_status ret;
    ni_id dir;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* see if the directory exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    return ni2_destroydirval(domain, &dir, key, values);
}

ni_status ni2_destroydirval(void *domain, ni_id *dir, const ni_name key, ni_namelist values)
{
    /* destroyval given a directory rather than a pathname */

    ni_status ret;
    ni_namelist nl;
    ni_index where, whereval;
    int i;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* fetch list of property keys from directory */
    NI_INIT(&nl);
    ret = ni_listprops(domain, dir, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* check for existing property with this key */
    where = ni_namelist_match(nl, key);
    ni_namelist_free(&nl);

    /* if property doesn't exist, nothing to do */
    if (where == NI_INDEX_NULL) {
        return NI_OK;
    }

    /* fetch existing namelist for this property */
    NI_INIT(&nl);
    ret = ni_readprop(domain, dir, where, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* delete values */
    for (i = 0; i < values.ni_namelist_len; i++) {
        whereval = ni_namelist_match(nl, values.ni_namelist_val[i]);
        while (whereval != NI_INDEX_NULL) {
            ni_namelist_delete(&nl, whereval);
            whereval = ni_namelist_match(nl, values.ni_namelist_val[i]);
        }
    }

    /* write the new list back */
    ret = ni_writeprop(domain, dir, where, nl);
    ni_namelist_free(&nl);

    return ret;
}

ni_status ni2_renameprop(void *domain, char *pathname, const ni_name oldname, const ni_name newname)
{
    /* rename a property */

    ni_status ret;
    ni_id dir;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* see if the directory already exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    return ni2_renamedirprop(domain, &dir, oldname, newname);
}

ni_status ni2_renamedirprop(void *domain, ni_id *dir, const ni_name oldname, const ni_name newname)
{
    /* renameprop given a directory rather than a pathname */

    ni_status ret;
    ni_index where;
    ni_namelist nl;

    /* need to be talking to the master */
    ni_needwrite(domain, 1);

    /* fetch list of property keys from directory */
    NI_INIT(&nl);
    ret = ni_listprops(domain, dir, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* look up old name */
    where = ni_namelist_match(nl, oldname);
    ni_namelist_free(&nl);

    /* if it's not there, return an error */
    if (where == NI_INDEX_NULL) {
        return NI_NOPROP;
    }

    return ni_renameprop(domain, dir, where, newname);
}

ni_status ni2_pathsearch(void *domain, ni_id *dir, char *pathname)
{
    /* same as pathsearch, but if pathname is an integer */
    /* then use it as a directory id */

    int i, len;
    bool is_id;

    len = strlen(pathname);
    is_id = true;

    for (i = 0; i < len && is_id; i++)
        if (!isdigit(pathname[i])) is_id = false;

    if (is_id) {
        dir->nii_object = (UInt32)atoi(pathname);
        return ni_self(domain, dir);
    }
    else {
        return ni_pathsearch(domain, dir, pathname);
    }
}

ni_status ni2_createpath(void *domain, ni_id *dir, char *pathname)
{
    /* make a directory with the given pathname */

    ni_status ret;
    ni_id checkdir;
    int i, j, len;
    char *dirname = NULL;
    bool simple;

    /* pull out every pathname component and create the directory */
    i = 0;
    while (pathname[i] != '\0') {

        /* search forward for a path component (a directory) */
        simple = true;
        for (j = i; pathname[j] != '\0' && simple; j++) {
            if (pathname[j] == '\\' && pathname[j+1] == '/') j+=2;
            if (pathname[j] == '/') simple = false;
        }

        len = j - i;
        if (!simple) len--;
        dirname = malloc(len + 1);
        strncpy(dirname, pathname+i, len);
        dirname[len] = '\0';

        /* advance the pointer */
        i = j;

        /* does this directory exist? */
        checkdir = *dir;
        ret = ni_pathsearch(domain, dir, dirname);

        /* if it doesn't exist, create it */
        if (ret == NI_NODIR) {
            *dir = checkdir;
            ret = ni2_createchild(domain, dir, dirname);
            if (ret != NI_OK) return ret;
        }
        free(dirname);
    }

    return NI_OK;
}

ni_status ni2_createchild(void *domain, ni_id *dir, const ni_name dirname)
{
    /* make a child directory with the given name */

    ni_status ret;
    ni_proplist p;
    ni_id child;
    int i, j, k, len;
    char *key = NULL;
    char *value = NULL;

    /* if the name contains "=", then we've got "foo=bar" */
    /* property key is "foo", not "name" */

    len = 0;
    for (i = 0; dirname[i] != '\0' && dirname[i] != '='; i++);
    if (dirname[i] == '=') len = i;

    if (len > 0) {
        key = malloc(len + 1);
        /* check for backslashes in property key */
        for (i = 0, j = 0; i < len; i++, j++) {
            if (dirname[i] == '\\' && dirname[i+1] == '/') i++;
            key[j] = dirname[i];
        }
        key[j] = '\0';
        i = len + 1;
    }
    else {
        key = malloc(5);
        strcpy(key, "name");
        i = 0;
    }

    /* compress out backslashes in value */
    j = strlen(dirname);
    len = j - i;
    value = malloc(len + 1);
    for (k = 0; i < j; k++, i++) {
        if (dirname[i] == '\\' && dirname[i+1] == '/') i++;
        value[k] = dirname[i];
    }
    value[k] = '\0';
        
    /* set up the new directory */
    NI_INIT(&p);
    nipl_createprop(&p, key);
    nipl_appendprop(&p, key, value);

    /* create it */
    ret = ni_create(domain, dir, p, &child, NI_INDEX_NULL);
    if (ret != NI_OK) {
        ni_proplist_free(&p);
        return ret;
    }

    ni_proplist_free(&p);
    free(key);
    free(value);
    *dir = child;
    return NI_OK;
}

void nipl_createprop(ni_proplist *l, const ni_name n)
{
    /* property list utility */
    /* add a name property to a property list */

    ni_property p;

    NI_INIT(&p);
    p.nip_name = ni_name_dup(n);
    p.nip_val.ninl_len = 0;
    p.nip_val.ninl_val = NULL;
    ni_proplist_insert(l, p, NI_INDEX_NULL);
    ni_prop_free(&p);
}

void nipl_appendprop(ni_proplist *l, const ni_name n, const ni_name v)
{
    /* property list utility */
    /* append a value to a property in a property list */

    ni_index where;

    where = ni_proplist_match(*l, n, NULL);
    if (where == NI_INDEX_NULL) {
        nipl_createprop(l, n);
        where = ni_proplist_match(*l, n, NULL);
    }
    ni_namelist_insert(&(l->nipl_val[where].nip_val), v, NI_INDEX_NULL);
}

void nipl_mergeprop(ni_proplist *l, const ni_name n, const ni_name v)
{
    /* property list utility */
    /* merge a value into a property in a property list */

    ni_index where;

    where = ni_proplist_match(*l, n, NULL);
    if (where == NI_INDEX_NULL) {
        nipl_createprop(l, n);
        where = ni_proplist_match(*l, n, NULL);
    }
    where = ni_namelist_match(l->nipl_val[where].nip_val, v);
    if (where == NI_INDEX_NULL) {
        ni_namelist_insert(&(l->nipl_val[where].nip_val), v, NI_INDEX_NULL);
    }
}

ni_status ni2_statprop(void *domain, char *pathname, const ni_name key, ni_index *where)
{
    /* match a property in a given property in a given directory */

    ni_status ret;
    ni_id dir;

    /* assume there's no match */
    *where = NI_INDEX_NULL;

    /* see if the directory exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    return ni2_statpropdir(domain, &dir, key, where);
}

ni_status ni2_statpropdir(void *domain, ni_id *dir, const ni_name key, ni_index *where)
{
    /* statprop given a directory rather than a pathname */

    ni_status ret;
    ni_namelist nl;

    /* assume there's no match */
    *where = NI_INDEX_NULL;

    /* fetch list of property keys from directory */
    NI_INIT(&nl);
    ret = ni_listprops(domain, dir, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* check for property with this key */
    *where = ni_namelist_match(nl, key);
    ni_namelist_free(&nl);

    /* if property doesn't exist, no match */
    if (*where == NI_INDEX_NULL) {
        return NI_NOPROP;
    }

    return NI_OK;
}

ni_status ni2_statval(void *domain, char *pathname, const ni_name key, const ni_name value, ni_index *where)
{
    /* match a value in a given property in a given directory */

    ni_status ret;
    ni_id dir;

    /* assume there's no match */
    *where = NI_INDEX_NULL;

    /* see if the directory exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    return ni2_statvaldir(domain, &dir, key, value, where);
}

ni_status ni2_statvaldir(void *domain, ni_id *dir, const ni_name key, const ni_name value, ni_index *where)
{
    /* statval given a directory rather than a pathname */

    ni_status ret;
    ni_namelist nl;
    ni_index wh;

    /* assume there's no match */
    *where = NI_INDEX_NULL;

    /* fetch list of property keys from directory */
    NI_INIT(&nl);
    ret = ni_listprops(domain, dir, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* check for property with this key */
    wh = ni_namelist_match(nl, key);
    ni_namelist_free(&nl);

    /* if property doesn't exist, no match */
    if (wh == NI_INDEX_NULL) {
        return NI_NOPROP;
    }

    /* fetch existing namelist for this property */
    NI_INIT(&nl);
    ret = ni_readprop(domain, dir, wh, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* check for this value */
    wh = ni_namelist_match(nl, value);
    ni_namelist_free(&nl);

    /* if value doesn't exist, no match */
    if (wh == NI_INDEX_NULL) {
        return NI_NONAME;
    }

    *where = wh;
    return NI_OK;
}

ni_status ni2_reapprop(void *domain, char *pathname, const ni_name key)
{
    /* remove a property in a given directory if the property is empty */

    ni_status ret;
    ni_id dir;

    /* see if the directory exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    return ni2_reappropdir(domain, &dir, key);
}

ni_status ni2_reappropdir(void *domain, ni_id *dir, const ni_name key)
{
    /* reapprop given a directory rather than a pathname */

    ni_status ret;
    ni_namelist nl;
    ni_index where;

    /* fetch list of property keys from directory */
    NI_INIT(&nl);
    ret = ni_listprops(domain, dir, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* check for property with this key */
    where = ni_namelist_match(nl, key);
    ni_namelist_free(&nl);

    /* if property doesn't exist, return */
    if (where == NI_INDEX_NULL) {
        return NI_OK;
    }

    /* fetch existing namelist for this property */
    NI_INIT(&nl);
    ret = ni_readprop(domain, dir, where, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* if the property contains any values, leave it alone */
    if (nl.ni_namelist_len > 0) {
        ni_namelist_free(&nl);
        return NI_OK;
    }

    /* property is empty, delete it */
    ni_namelist_free(&nl);
    return ni_destroyprop(domain, dir, where);
}

ni_status ni2_reapdir(void *domain, char *pathname)
{
    /* destroy a directory if it has nothing but a name */

    ni_status ret;
    ni_id dir;
    ni_namelist nl;

    /* see if the directory exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    /* fetch list of property keys from directory */
    NI_INIT(&nl);
    ret = ni_listprops(domain, &dir, &nl);
    if (ret != NI_OK) {
        return ret;
    }

    /* if more than one property, leave it alone */
    if (nl.ni_namelist_len > 1) {
        ni_namelist_free(&nl);
        return NI_OK;
    }

    /* directory is empty (except for name), delete it */
    ni_namelist_free(&nl);
    return ni2_destroy(domain, pathname);
}

ni_status ni2_copy(void *srcdomain, char *path, void*dstdomain, bool recursive)
{
    /* copy a directory from src to dst */

    ni_status ret;
    ni_id srcdir, dstdir;

    /* see if src directory exists */
    ret = ni2_pathsearch(srcdomain, &srcdir, path);
    if (ret != NI_OK) return ret;

    /* create dstdir if necessary */
    ret = ni2_create(dstdomain, path);
    if (ret != NI_OK) return ret;

    /* get dstdir */
    ret = ni2_pathsearch(dstdomain, &dstdir, path);
    if (ret != NI_OK) return ret;

    return ni2_copydir(srcdomain, &srcdir, dstdomain, &dstdir, recursive);
}

ni_status ni2_copydir(void *srcdomain, ni_id *srcdir, void*dstdomain, ni_id *dstdir , bool recursive)
{
    ni_status ret;
    ni_idlist children;
    int i, len;
    ni_proplist p;
    ni_id dir;

    NI_INIT(&p);
    
    /* get proplist from src dir */
    ret = ni_read(srcdomain, srcdir, &p);
    if (ret != NI_OK) {
        return ret;
    }

    /* write the property list to the dst dir */
    ret = ni_write(dstdomain, dstdir, p);
    if (ret != NI_OK) {
        ni_proplist_free(&p);
        return ret;
    }
    
    ni_proplist_free(&p);

    if (recursive) {
        NI_INIT(&children);

        /* get list of children */
        ret = ni_children(srcdomain, srcdir, &children);
        if (ret != NI_OK) {
            return ret;
        }

        len = children.ni_idlist_len;
        for (i = 0; i < len; i++) {
            dir.nii_object = children.ni_idlist_val[i];
            ret = ni_self(srcdomain, &dir);
            if (ret != NI_OK) {
                ni_idlist_free(&children);
                return ret;
            }
            ret = ni2_copydirtoparentdir(srcdomain,&dir,dstdomain,dstdir,recursive);
        }
    
        ni_idlist_free(&children);
    }

    return NI_OK;
}

ni_status ni2_copydirtoparentdir(void *srcdomain, ni_id *srcdir, void*dstdomain, ni_id *dstdir , bool recursive)
{
    ni_status ret;
    ni_idlist children;
    int i, len;
    ni_proplist p;
    ni_id dir, newdstdir;

    NI_INIT(&p);
    
    /* get proplist from src dir */
    ret = ni_read(srcdomain, srcdir, &p);
    if (ret != NI_OK) {
        return ret;
    }

    /* create the destination dir */
    ret = ni_create(dstdomain, dstdir, p, &newdstdir, NI_INDEX_NULL);
    if (ret != NI_OK) {
        ni_proplist_free(&p);
        return ret;
    }
    
    ni_proplist_free(&p);

    if (recursive) {
        NI_INIT(&children);

        /* get list of children */
        ret = ni_children(srcdomain, srcdir, &children);
        if (ret != NI_OK) {
            return ret;
        }

        len = children.ni_idlist_len;
        for (i = 0; i < len; i++) {
            dir.nii_object = children.ni_idlist_val[i];
            ret = ni_self(srcdomain, &dir);
            if (ret != NI_OK) {
                ni_idlist_free(&children);
                return ret;
            }
            ret = ni2_copydirtoparentdir(srcdomain,&dir,dstdomain, &newdstdir,recursive);
        }
    
        ni_idlist_free(&children);
    }

    return NI_OK;
}

ni_status ni2_lookupprop(void *domain, char *pathname, const ni_name key, ni_namelist *values)
{
    /* read a property */

    ni_status ret;
    ni_id dir;

    /* see if the directory exists */
    ret = ni2_pathsearch(domain, &dir, pathname);
    if (ret != NI_OK) return ret;

    return ni_lookupprop(domain, &dir, key, values);
}

ni_index ni_namelist_insert_sorted(ni_namelist *values, const ni_name newvalue)
{
    int i, len;

    len = values->ni_namelist_len;
    for (i = 0; i < len; i++) {
        if (strcmp(newvalue, values->ni_namelist_val[i]) <= 0) {
            ni_namelist_insert(values, newvalue, (ni_index)i);
            return (ni_index)i;
        }
    }

    ni_namelist_insert(values, newvalue, NI_INDEX_NULL);
    return NI_INDEX_NULL;
}
