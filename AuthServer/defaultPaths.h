/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
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

# define DEFAULTPATHS_ETC_DIR			"c:\\Program Files\\EasyDSS\\"
# define DEFAULTPATHS_ETC_DIR_OLD		"c:\\Program Files\\EasyDSS\\"
# define DEFAULTPATHS_SSM_DIR			"c:\\Program Files\\EasyDSS\\QTSSModules\\"
# define DEFAULTPATHS_LOG_DIR			"c:\\Program Files\\EasyDSS\\Logs\\"
# define DEFAULTPATHS_PID_FILE          ""
# define DEFAULTPATHS_PID_DIR			""
#elif __MacOSX__

# define DEFAULTPATHS_DIRECTORY_SEPARATOR	"/"

# define DEFAULTPATHS_ETC_DIR			"/opt/streaming/Config/"
# define DEFAULTPATHS_ETC_DIR_OLD		"/opt/"
# define DEFAULTPATHS_SSM_DIR			"/opt/streaming/Modules/"
# define DEFAULTPATHS_LOG_DIR			"/opt/streaming/Logs/"
# define DEFAULTPATHS_PID_DIR           "/var/run/"

#else

# define DEFAULTPATHS_DIRECTORY_SEPARATOR	"/"

# define DEFAULTPATHS_ETC_DIR			"/opt/streaming/"
# define DEFAULTPATHS_ETC_DIR_OLD		"/opt/"
# define DEFAULTPATHS_SSM_DIR			"/usr/local/sbin/StreamingServerModules/"
# define DEFAULTPATHS_LOG_DIR			"/var/streaming/logs/"
# define DEFAULTPATHS_MOVIES_DIR        	"/usr/local/movies/"
# define DEFAULTPATHS_PID_DIR           		"/var/run/"

#endif
