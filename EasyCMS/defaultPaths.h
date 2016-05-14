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

# define DEFAULTPATHS_ETC_DIR			"c:\\Program Files\\EasyCMS\\"
# define DEFAULTPATHS_ETC_DIR_OLD		"c:\\Program Files\\EasyCMS\\"
# define DEFAULTPATHS_SSM_DIR			"c:\\Program Files\\EasyCMS\\QTSSModules\\"
# define DEFAULTPATHS_LOG_DIR			"c:\\Program Files\\EasyCMS\\Logs\\"
# define DEFAULTPATHS_PID_FILE          ""
# define DEFAULTPATHS_PID_DIR			""
#elif __MacOSX__

# define DEFAULTPATHS_DIRECTORY_SEPARATOR	"/"

# define DEFAULTPATHS_ETC_DIR			"/opt/streaming/EasyCMS/Config/"
# define DEFAULTPATHS_ETC_DIR_OLD		"/opt/EasyCMS/"
# define DEFAULTPATHS_SSM_DIR			"/opt/streaming/EasyCMS/Modules/"
# define DEFAULTPATHS_LOG_DIR			"/opt/streaming/EasyCMS/Logs/"
# define DEFAULTPATHS_PID_DIR           "/var/run/EasyCMS/"

#else

# define DEFAULTPATHS_DIRECTORY_SEPARATOR	"/"

# define DEFAULTPATHS_ETC_DIR			"/opt/streaming/EasyCMS/"
# define DEFAULTPATHS_ETC_DIR_OLD		"/opt/EasyCMS/"
# define DEFAULTPATHS_SSM_DIR			"/usr/local/sbin/EasyCMSModules/"
# define DEFAULTPATHS_LOG_DIR			"/var/streaming/EasyCMS/logs/"
# define DEFAULTPATHS_MOVIES_DIR        "/usr/local/EasyCMS/movies/"
# define DEFAULTPATHS_PID_DIR           "/var/run/EasyCMS/"

#endif
