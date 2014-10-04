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
 * get_opt.c
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#if defined(mac)
#include <types.h>
#endif

int get_opt(int argc, char *argv[], char *optList);

char *optarg = NULL;
char *whatOption = NULL;
int currentOpt = 0;
int optind = 1;
extern char gOptionsChar;

/* compareOptionString takes an option parameter with the following format:
 *  -REQuired  : the characters in CAPS are required to match.
 */
static int compareOptionString(char *option, char *compareit) {
    int i, l1, l2;

    l1 = strlen(option);
    l2 = strlen(compareit);
    for (i=0; i<l2; i++) {
        if (option[i] == '\0')
            return -1;          /* option < compareit */
        if (tolower(option[i]) == tolower(compareit[i])) {
        }
        else {
            if (option[i] > compareit[i])
                return 1;       /* option > compareit */
            else
                return -1;      /* option < compareit */
        }
    }
    /* if we got here, all of the characters in compareit are in option. */
    /* now we've got to check if there are any more required characters. */
    if (l1 > l2) {          /* only need to check if option is longer than compareit */
        if (option[l2] == toupper(option[l2]))
            return 1;       /* there was an additional character[s] needed. */
    }

    /* if we got here, then all is hunky dory and we got the required stuff. */
    return 0;
}

int get_opt(int argc, char *argv[], char *optList) {
    char    option[256];
    int     l, i, c, opt;

    currentOpt = optind;
    optind++;
    if (currentOpt >= argc) {
        return EOF;
    }
    optarg = NULL;
    whatOption = argv[currentOpt];

    if (whatOption[0] != gOptionsChar) {
        optarg = whatOption;
        return 0;
    }
    l = strlen(optList);
    i = 1;
    c = 0;
    opt = 1;
    option[c++] = gOptionsChar;
    while (i<=l) {
        if (optList[i] == gOptionsChar) {
            option[c++] = '\0';
//          if (strncasecmp(option, whatOption, strlen(whatOption)) == 0)
            if (compareOptionString(option, whatOption) == 0)
//              return opt;
                return option[1];
            c = 1;      // reset option string to '-' since we've just seen it
            opt++;      // check next option
        }
        else if (optList[i] == ':') {
            option[c++] = '\0';
//          if (strncasecmp(option, whatOption, strlen(whatOption)) == 0) {
            if (compareOptionString(option, whatOption) == 0) {
                currentOpt++;   optind++;
                optarg = argv[currentOpt];
//              return opt;
                return option[1];
            }
            c = 1;      // reset option string to '-'
            opt++;      // check next option
            i++;        // pass over the :
            if (optList[i] == gOptionsChar) {
                   ;            // this is where we want to be
            }
            else if (optList[i] == '\0') {
//              return EOF;     // this was the last option to check
                return 0;       // this was the last option to check
            }
            else {
                fprintf(stderr, "Malformed getopt string '%s': character %d is '%c' was expecting %c\n", optList, i, optList[i], gOptionsChar);
                return 0;
            }

        }
        else if (optList[i] == '\0') {
            option[c++] = '\0';
//          if (strncasecmp(option, whatOption, strlen(whatOption)) == 0) {
            if (compareOptionString(option, whatOption) == 0) {
//              return opt;
                return option[1];
            }
            optarg = argv[currentOpt];
//          return EOF;     // didn't find anything, return EOF
            return 0;       // didn't find anything, return EOF
        }
        else {
            option[c++] = optList[i];
        }
        i++;
    }

    optarg = argv[currentOpt];
    return 0;
}

