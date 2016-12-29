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
CND_PLATFORM=GNU-Linux
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
	${OBJECTDIR}/_ext/b9fc5c32/HTTPProtocol.o \
	${OBJECTDIR}/_ext/b9fc5c32/HTTPRequest.o \
	${OBJECTDIR}/_ext/dfc17a71/OSMemory.o \
	${OBJECTDIR}/_ext/cbf2331e/InternalStdLib.o \
	${OBJECTDIR}/APICommonCode/QTAccessFile.o \
	${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o \
	${OBJECTDIR}/APICommonCode/QTSSRollingLog.o \
	${OBJECTDIR}/APIModules/EasyRedisModule/EasyRedisModule.o \
	${OBJECTDIR}/APIStubLib/QTSS_Private.o \
	${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o \
	${OBJECTDIR}/PrefsSourceLib/XMLParser.o \
	${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o \
	${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o \
	${OBJECTDIR}/Server.tproj/HTTPRequestStream.o \
	${OBJECTDIR}/Server.tproj/HTTPResponseStream.o \
	${OBJECTDIR}/Server.tproj/HTTPSession.o \
	${OBJECTDIR}/Server.tproj/HTTPSessionInterface.o \
	${OBJECTDIR}/Server.tproj/QTSSCallbacks.o \
	${OBJECTDIR}/Server.tproj/QTSSDataConverter.o \
	${OBJECTDIR}/Server.tproj/QTSSDictionary.o \
	${OBJECTDIR}/Server.tproj/QTSSErrorLogModule.o \
	${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o \
	${OBJECTDIR}/Server.tproj/QTSSFile.o \
	${OBJECTDIR}/Server.tproj/QTSSMessages.o \
	${OBJECTDIR}/Server.tproj/QTSSModule.o \
	${OBJECTDIR}/Server.tproj/QTSSPrefs.o \
	${OBJECTDIR}/Server.tproj/QTSSSocket.o \
	${OBJECTDIR}/Server.tproj/QTSServer.o \
	${OBJECTDIR}/Server.tproj/QTSServerInterface.o \
	${OBJECTDIR}/Server.tproj/QTSServerPrefs.o \
	${OBJECTDIR}/Server.tproj/RunServer.o \
	${OBJECTDIR}/Server.tproj/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../CommonUtilitiesLib/${CND_CONF} -L../EasyProtocol/EasyProtocol/${CND_CONF} -L../EasyProtocol/jsoncpp/${CND_CONF} -L../Lib/x64 -L../EasyRedisClient/${CND_CONF} -L../Lib/FFmpeg/linux/x64

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_CONF}/easycms

${CND_CONF}/easycms: ${OBJECTFILES}
	${MKDIR} -p ${CND_CONF}
	${LINK.cc} -o ${CND_CONF}/easycms ${OBJECTFILES} ${LDLIBSOPTIONS} -lCommonUtilitiesLib -ldl -lpthread -lEasyProtocol -ljsoncpp -lboost_system -lboost_thread -lrt -leasyredisclient -lavformat -lavcodec -lavutil -lavfilter -lswscale -lavdevice -lswresample

${OBJECTDIR}/_ext/b9fc5c32/HTTPProtocol.o: ../HTTPUtilitiesLib/HTTPProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/b9fc5c32
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b9fc5c32/HTTPProtocol.o ../HTTPUtilitiesLib/HTTPProtocol.cpp

${OBJECTDIR}/_ext/b9fc5c32/HTTPRequest.o: ../HTTPUtilitiesLib/HTTPRequest.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/b9fc5c32
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b9fc5c32/HTTPRequest.o ../HTTPUtilitiesLib/HTTPRequest.cpp

${OBJECTDIR}/_ext/dfc17a71/OSMemory.o: ../OSMemoryLib/OSMemory.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/dfc17a71
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/dfc17a71/OSMemory.o ../OSMemoryLib/OSMemory.cpp

${OBJECTDIR}/_ext/cbf2331e/InternalStdLib.o: ../SafeStdLib/InternalStdLib.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/cbf2331e
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cbf2331e/InternalStdLib.o ../SafeStdLib/InternalStdLib.cpp

${OBJECTDIR}/APICommonCode/QTAccessFile.o: APICommonCode/QTAccessFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTAccessFile.o APICommonCode/QTAccessFile.cpp

${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o: APICommonCode/QTSSModuleUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o APICommonCode/QTSSModuleUtils.cpp

${OBJECTDIR}/APICommonCode/QTSSRollingLog.o: APICommonCode/QTSSRollingLog.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTSSRollingLog.o APICommonCode/QTSSRollingLog.cpp

${OBJECTDIR}/APIModules/EasyRedisModule/EasyRedisModule.o: APIModules/EasyRedisModule/EasyRedisModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/EasyRedisModule
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/EasyRedisModule/EasyRedisModule.o APIModules/EasyRedisModule/EasyRedisModule.cpp

${OBJECTDIR}/APIStubLib/QTSS_Private.o: APIStubLib/QTSS_Private.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIStubLib
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIStubLib/QTSS_Private.o APIStubLib/QTSS_Private.cpp

${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o: PrefsSourceLib/FilePrefsSource.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o PrefsSourceLib/FilePrefsSource.cpp

${OBJECTDIR}/PrefsSourceLib/XMLParser.o: PrefsSourceLib/XMLParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLParser.o PrefsSourceLib/XMLParser.cpp

${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o: PrefsSourceLib/XMLPrefsParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o PrefsSourceLib/XMLPrefsParser.cpp

${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o: Server.tproj/GenerateXMLPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o Server.tproj/GenerateXMLPrefs.cpp

${OBJECTDIR}/Server.tproj/HTTPRequestStream.o: Server.tproj/HTTPRequestStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/HTTPRequestStream.o Server.tproj/HTTPRequestStream.cpp

${OBJECTDIR}/Server.tproj/HTTPResponseStream.o: Server.tproj/HTTPResponseStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/HTTPResponseStream.o Server.tproj/HTTPResponseStream.cpp

${OBJECTDIR}/Server.tproj/HTTPSession.o: Server.tproj/HTTPSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/HTTPSession.o Server.tproj/HTTPSession.cpp

${OBJECTDIR}/Server.tproj/HTTPSessionInterface.o: Server.tproj/HTTPSessionInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/HTTPSessionInterface.o Server.tproj/HTTPSessionInterface.cpp

${OBJECTDIR}/Server.tproj/QTSSCallbacks.o: Server.tproj/QTSSCallbacks.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSCallbacks.o Server.tproj/QTSSCallbacks.cpp

${OBJECTDIR}/Server.tproj/QTSSDataConverter.o: Server.tproj/QTSSDataConverter.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSDataConverter.o Server.tproj/QTSSDataConverter.cpp

${OBJECTDIR}/Server.tproj/QTSSDictionary.o: Server.tproj/QTSSDictionary.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSDictionary.o Server.tproj/QTSSDictionary.cpp

${OBJECTDIR}/Server.tproj/QTSSErrorLogModule.o: Server.tproj/QTSSErrorLogModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSErrorLogModule.o Server.tproj/QTSSErrorLogModule.cpp

${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o: Server.tproj/QTSSExpirationDate.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o Server.tproj/QTSSExpirationDate.cpp

${OBJECTDIR}/Server.tproj/QTSSFile.o: Server.tproj/QTSSFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSFile.o Server.tproj/QTSSFile.cpp

${OBJECTDIR}/Server.tproj/QTSSMessages.o: Server.tproj/QTSSMessages.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSMessages.o Server.tproj/QTSSMessages.cpp

${OBJECTDIR}/Server.tproj/QTSSModule.o: Server.tproj/QTSSModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSModule.o Server.tproj/QTSSModule.cpp

${OBJECTDIR}/Server.tproj/QTSSPrefs.o: Server.tproj/QTSSPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSPrefs.o Server.tproj/QTSSPrefs.cpp

${OBJECTDIR}/Server.tproj/QTSSSocket.o: Server.tproj/QTSSSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSSocket.o Server.tproj/QTSSSocket.cpp

${OBJECTDIR}/Server.tproj/QTSServer.o: Server.tproj/QTSServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServer.o Server.tproj/QTSServer.cpp

${OBJECTDIR}/Server.tproj/QTSServerInterface.o: Server.tproj/QTSServerInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerInterface.o Server.tproj/QTSServerInterface.cpp

${OBJECTDIR}/Server.tproj/QTSServerPrefs.o: Server.tproj/QTSServerPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerPrefs.o Server.tproj/QTSServerPrefs.cpp

${OBJECTDIR}/Server.tproj/RunServer.o: Server.tproj/RunServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RunServer.o Server.tproj/RunServer.cpp

${OBJECTDIR}/Server.tproj/main.o: Server.tproj/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -g -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyAuthModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../Include/FFmpeg/linux -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/main.o Server.tproj/main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_CONF}/easycms

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
