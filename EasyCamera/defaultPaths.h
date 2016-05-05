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
 * defaultPaths.h - define the default paths to hardcode into the executables
 * 
 * IMPORTANT NOTE : The trailering directory separators are required on all 
 *                  DEFAULTPATHS_*_DIR* defines
 *
 * Contributed by: Peter Bray
 */

#ifdef __Win32__

# define DEFAULTPATHS_DIRECTORY_SEPARATOR	"\\"

# define DEFAULTPATHS_ETC_DIR			"./"
# define DEFAULTPATHS_ETC_DIR_OLD		"./"
# define DEFAULTPATHS_SSM_DIR			"./QTSSModules/"
# define DEFAULTPATHS_LOG_DIR			"./Logs/"
# define DEFAULTPATHS_PID_FILE          ""
# define DEFAULTPATHS_PID_DIR			""
#elif __MacOSX__

# define DEFAULTPATHS_DIRECTORY_SEPARATOR	"/"

# define DEFAULTPATHS_ETC_DIR			"/Library/QuickTimeStreaming/Config/"
# define DEFAULTPATHS_ETC_DIR_OLD		"/etc/"
# define DEFAULTPATHS_SSM_DIR			"/Library/QuickTimeStreaming/Modules/"
# define DEFAULTPATHS_LOG_DIR			"/Library/QuickTimeStreaming/Logs/"
# define DEFAULTPATHS_PID_DIR           "/var/run/"

#else

# define DEFAULTPATHS_DIRECTORY_SEPARATOR	"/"

# define DEFAULTPATHS_ETC_DIR			"/etc/streaming/"
# define DEFAULTPATHS_ETC_DIR_OLD		"/etc/"
# define DEFAULTPATHS_SSM_DIR			"/usr/local/sbin/StreamingServerModules/"
# define DEFAULTPATHS_LOG_DIR			"/var/streaming/logs/"
# define DEFAULTPATHS_PID_DIR           "/var/run/"

#endif
