#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1321107971/QTAccessFile.o \
	${OBJECTDIR}/_ext/1321107971/QTSSModuleUtils.o \
	${OBJECTDIR}/_ext/1321107971/QTSSRollingLog.o \
	${OBJECTDIR}/_ext/1321107971/SDPSourceInfo.o \
	${OBJECTDIR}/_ext/1321107971/SourceInfo.o \
	${OBJECTDIR}/_ext/2115519668/QTSS_Private.o \
	${OBJECTDIR}/_ext/1174643662/HTTPProtocol.o \
	${OBJECTDIR}/_ext/1174643662/HTTPRequest.o \
	${OBJECTDIR}/_ext/1103271215/OSMemory.o \
	${OBJECTDIR}/_ext/1103271215/QTSSDataConverter.o \
	${OBJECTDIR}/_ext/1103271215/QTSSErrorLogModule.o \
	${OBJECTDIR}/_ext/1103271215/QTSSUserProfile.o \
	${OBJECTDIR}/_ext/1103271215/SysUtil.o \
	${OBJECTDIR}/_ext/1103271215/commonfunc.o \
	${OBJECTDIR}/APIModules/QTSSAccessModule/AccessChecker.o \
	${OBJECTDIR}/APIModules/QTSSAccessModule/QTSSAccessModule.o \
	${OBJECTDIR}/APIModules/QTSSPOSIXFileSysModule/QTSSPosixFileSysModule.o \
	${OBJECTDIR}/APIModules/QTSSWebDebugModule/QTSSWebDebugModule.o \
	${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o \
	${OBJECTDIR}/PrefsSourceLib/XMLParser.o \
	${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o \
	${OBJECTDIR}/Server.tproj/BaseRequestInterface.o \
	${OBJECTDIR}/Server.tproj/BaseRequestStream.o \
	${OBJECTDIR}/Server.tproj/BaseResponseStream.o \
	${OBJECTDIR}/Server.tproj/BaseSessionInterface.o \
	${OBJECTDIR}/Server.tproj/DispatchMsgCenter.o \
	${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o \
	${OBJECTDIR}/Server.tproj/QTSSCallbacks.o \
	${OBJECTDIR}/Server.tproj/QTSSDictionary.o \
	${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o \
	${OBJECTDIR}/Server.tproj/QTSSFile.o \
	${OBJECTDIR}/Server.tproj/QTSSMessages.o \
	${OBJECTDIR}/Server.tproj/QTSSModule.o \
	${OBJECTDIR}/Server.tproj/QTSSPrefs.o \
	${OBJECTDIR}/Server.tproj/QTSSSocket.o \
	${OBJECTDIR}/Server.tproj/QTSServer.o \
	${OBJECTDIR}/Server.tproj/QTSServerInterface.o \
	${OBJECTDIR}/Server.tproj/QTSServerPrefs.o \
	${OBJECTDIR}/Server.tproj/RTSPProtocol.o \
	${OBJECTDIR}/Server.tproj/RedisSession.o \
	${OBJECTDIR}/Server.tproj/RunServer.o \
	${OBJECTDIR}/Server.tproj/ServiceSession.o \
	${OBJECTDIR}/Server.tproj/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-pipe -Wno-format-y2k
CXXFLAGS=-pipe -Wno-format-y2k

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L${AVS_ROOT}/lib

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_CONF}/dms

${CND_CONF}/dms: ${OBJECTFILES}
	${MKDIR} -p ${CND_CONF}
	${LINK.cc} -o ${CND_CONF}/dms ${OBJECTFILES} ${LDLIBSOPTIONS} -lCommonUtilitiesLib -lEasyDSSProtocol -lavsxmlutil -lTinyDispatchEvent -lavsredisclient -lpthread -ldl

${OBJECTDIR}/_ext/1321107971/QTAccessFile.o: ../APICommonCode/QTAccessFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1321107971
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1321107971/QTAccessFile.o ../APICommonCode/QTAccessFile.cpp

${OBJECTDIR}/_ext/1321107971/QTSSModuleUtils.o: ../APICommonCode/QTSSModuleUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1321107971
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1321107971/QTSSModuleUtils.o ../APICommonCode/QTSSModuleUtils.cpp

${OBJECTDIR}/_ext/1321107971/QTSSRollingLog.o: ../APICommonCode/QTSSRollingLog.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1321107971
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1321107971/QTSSRollingLog.o ../APICommonCode/QTSSRollingLog.cpp

${OBJECTDIR}/_ext/1321107971/SDPSourceInfo.o: ../APICommonCode/SDPSourceInfo.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1321107971
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1321107971/SDPSourceInfo.o ../APICommonCode/SDPSourceInfo.cpp

${OBJECTDIR}/_ext/1321107971/SourceInfo.o: ../APICommonCode/SourceInfo.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1321107971
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1321107971/SourceInfo.o ../APICommonCode/SourceInfo.cpp

${OBJECTDIR}/_ext/2115519668/QTSS_Private.o: ../APIStubLib/QTSS_Private.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2115519668
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2115519668/QTSS_Private.o ../APIStubLib/QTSS_Private.cpp

${OBJECTDIR}/_ext/1174643662/HTTPProtocol.o: ../HTTPUtilitiesLib/HTTPProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1174643662
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1174643662/HTTPProtocol.o ../HTTPUtilitiesLib/HTTPProtocol.cpp

${OBJECTDIR}/_ext/1174643662/HTTPRequest.o: ../HTTPUtilitiesLib/HTTPRequest.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1174643662
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1174643662/HTTPRequest.o ../HTTPUtilitiesLib/HTTPRequest.cpp

${OBJECTDIR}/_ext/1103271215/OSMemory.o: ../ServiceCommon/OSMemory.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1103271215
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1103271215/OSMemory.o ../ServiceCommon/OSMemory.cpp

${OBJECTDIR}/_ext/1103271215/QTSSDataConverter.o: ../ServiceCommon/QTSSDataConverter.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1103271215
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1103271215/QTSSDataConverter.o ../ServiceCommon/QTSSDataConverter.cpp

${OBJECTDIR}/_ext/1103271215/QTSSErrorLogModule.o: ../ServiceCommon/QTSSErrorLogModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1103271215
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1103271215/QTSSErrorLogModule.o ../ServiceCommon/QTSSErrorLogModule.cpp

${OBJECTDIR}/_ext/1103271215/QTSSUserProfile.o: ../ServiceCommon/QTSSUserProfile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1103271215
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1103271215/QTSSUserProfile.o ../ServiceCommon/QTSSUserProfile.cpp

${OBJECTDIR}/_ext/1103271215/SysUtil.o: ../ServiceCommon/SysUtil.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1103271215
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1103271215/SysUtil.o ../ServiceCommon/SysUtil.cpp

${OBJECTDIR}/_ext/1103271215/commonfunc.o: ../ServiceCommon/commonfunc.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1103271215
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1103271215/commonfunc.o ../ServiceCommon/commonfunc.cpp

${OBJECTDIR}/APIModules/QTSSAccessModule/AccessChecker.o: APIModules/QTSSAccessModule/AccessChecker.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSAccessModule
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSAccessModule/AccessChecker.o APIModules/QTSSAccessModule/AccessChecker.cpp

${OBJECTDIR}/APIModules/QTSSAccessModule/QTSSAccessModule.o: APIModules/QTSSAccessModule/QTSSAccessModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSAccessModule
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSAccessModule/QTSSAccessModule.o APIModules/QTSSAccessModule/QTSSAccessModule.cpp

${OBJECTDIR}/APIModules/QTSSPOSIXFileSysModule/QTSSPosixFileSysModule.o: APIModules/QTSSPOSIXFileSysModule/QTSSPosixFileSysModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSPOSIXFileSysModule
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSPOSIXFileSysModule/QTSSPosixFileSysModule.o APIModules/QTSSPOSIXFileSysModule/QTSSPosixFileSysModule.cpp

${OBJECTDIR}/APIModules/QTSSWebDebugModule/QTSSWebDebugModule.o: APIModules/QTSSWebDebugModule/QTSSWebDebugModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSWebDebugModule
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSWebDebugModule/QTSSWebDebugModule.o APIModules/QTSSWebDebugModule/QTSSWebDebugModule.cpp

${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o: PrefsSourceLib/FilePrefsSource.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o PrefsSourceLib/FilePrefsSource.cpp

${OBJECTDIR}/PrefsSourceLib/XMLParser.o: PrefsSourceLib/XMLParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLParser.o PrefsSourceLib/XMLParser.cpp

${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o: PrefsSourceLib/XMLPrefsParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o PrefsSourceLib/XMLPrefsParser.cpp

${OBJECTDIR}/Server.tproj/BaseRequestInterface.o: Server.tproj/BaseRequestInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/BaseRequestInterface.o Server.tproj/BaseRequestInterface.cpp

${OBJECTDIR}/Server.tproj/BaseRequestStream.o: Server.tproj/BaseRequestStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/BaseRequestStream.o Server.tproj/BaseRequestStream.cpp

${OBJECTDIR}/Server.tproj/BaseResponseStream.o: Server.tproj/BaseResponseStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/BaseResponseStream.o Server.tproj/BaseResponseStream.cpp

${OBJECTDIR}/Server.tproj/BaseSessionInterface.o: Server.tproj/BaseSessionInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/BaseSessionInterface.o Server.tproj/BaseSessionInterface.cpp

${OBJECTDIR}/Server.tproj/DispatchMsgCenter.o: Server.tproj/DispatchMsgCenter.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/DispatchMsgCenter.o Server.tproj/DispatchMsgCenter.cpp

${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o: Server.tproj/GenerateXMLPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o Server.tproj/GenerateXMLPrefs.cpp

${OBJECTDIR}/Server.tproj/QTSSCallbacks.o: Server.tproj/QTSSCallbacks.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSCallbacks.o Server.tproj/QTSSCallbacks.cpp

${OBJECTDIR}/Server.tproj/QTSSDictionary.o: Server.tproj/QTSSDictionary.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSDictionary.o Server.tproj/QTSSDictionary.cpp

${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o: Server.tproj/QTSSExpirationDate.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o Server.tproj/QTSSExpirationDate.cpp

${OBJECTDIR}/Server.tproj/QTSSFile.o: Server.tproj/QTSSFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSFile.o Server.tproj/QTSSFile.cpp

${OBJECTDIR}/Server.tproj/QTSSMessages.o: Server.tproj/QTSSMessages.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSMessages.o Server.tproj/QTSSMessages.cpp

${OBJECTDIR}/Server.tproj/QTSSModule.o: Server.tproj/QTSSModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSModule.o Server.tproj/QTSSModule.cpp

${OBJECTDIR}/Server.tproj/QTSSPrefs.o: Server.tproj/QTSSPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSPrefs.o Server.tproj/QTSSPrefs.cpp

${OBJECTDIR}/Server.tproj/QTSSSocket.o: Server.tproj/QTSSSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSSocket.o Server.tproj/QTSSSocket.cpp

${OBJECTDIR}/Server.tproj/QTSServer.o: Server.tproj/QTSServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServer.o Server.tproj/QTSServer.cpp

${OBJECTDIR}/Server.tproj/QTSServerInterface.o: Server.tproj/QTSServerInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerInterface.o Server.tproj/QTSServerInterface.cpp

${OBJECTDIR}/Server.tproj/QTSServerPrefs.o: Server.tproj/QTSServerPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerPrefs.o Server.tproj/QTSServerPrefs.cpp

${OBJECTDIR}/Server.tproj/RTSPProtocol.o: Server.tproj/RTSPProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTSPProtocol.o Server.tproj/RTSPProtocol.cpp

${OBJECTDIR}/Server.tproj/RedisSession.o: Server.tproj/RedisSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RedisSession.o Server.tproj/RedisSession.cpp

${OBJECTDIR}/Server.tproj/RunServer.o: Server.tproj/RunServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RunServer.o Server.tproj/RunServer.cpp

${OBJECTDIR}/Server.tproj/ServiceSession.o: Server.tproj/ServiceSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/ServiceSession.o Server.tproj/ServiceSession.cpp

${OBJECTDIR}/Server.tproj/main.o: Server.tproj/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -DUSE_POSIX -D_REENTRANT -D__linux__ -I../CommonUtilitiesLib -I../APIStubLib -I../HTTPUtilitiesLib -I../ServiceCommon -I../libTinyDispatchEvent/src -I../EasyDSSBase -I../include -I../include/EasyDSSProtocol -IAPIModules/QTSSAccessModule -I../APICommonCode -IPrefsSourceLib -IServer.tproj -I../libavsredisclient -IAPIModules/QTSSPOSIXFileSysModule -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/main.o Server.tproj/main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_CONF}/dms

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
