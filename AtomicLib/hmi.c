/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
 */
/*
 * to test use of timestamp()
 */

 /* cc hmi.c -o hmi libatomic.a */

#include <stdio.h>
#include "timestamp.h"

void
main(int argc, char **argv)
{
    int i, j;
    SInt64 ts1, ts2;
    SInt64 ts3, ts4;
    struct timescale tsc;
    double scale;

    if (argc <= 1) {
        qtss_fprintf(stderr, "Usage: %s loop-count\n", argv[0]);
        exit(1);
    }

    j = atoi(argv[1]);

    ts1 = timestamp();  /* START */

    /* Loop for the given loop-count */
    for (i=0; i < j; i++) {
        ;
    }

    ts2 = timestamp();  /* END */

    qtss_printf("ts1 = %qd, ts2 = %qd\n", ts1, ts2);

    utimescale(&tsc);
    scale = (double)tsc.tsc_numerator / (double)tsc.tsc_denominator;

    ts1 = (SInt64)((double)ts1 * scale);
    ts2 = (SInt64)((double)ts2 * scale);

    qtss_printf("ts1 = %qd, ts2 = %qd, micro seconds = %qd\n",
            ts1, ts2, (ts2 - ts1));

    /* Use the scaledtimestamp() now */

    ts3 = scaledtimestamp(scale);   /* START */

    /* Loop for the given loop-count */
    for (i=0; i < j; i++) {
        ;
    }

    ts4 = scaledtimestamp(scale);   /* END */

    qtss_printf("ts3 = %qd, ts4 = %qd, micro seconds = %qd\n",
            ts3, ts4, (ts4 - ts3));

}
