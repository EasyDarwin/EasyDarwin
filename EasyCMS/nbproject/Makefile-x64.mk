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
CND_CONF=x64
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
	${OBJECTDIR}/Server.tproj/RTSPProtocol.o \
	${OBJECTDIR}/Server.tproj/RunServer.o \
	${OBJECTDIR}/Server.tproj/main.o \
	${OBJECTDIR}/Server.tproj/QTSSAuthModule.o \


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

#uiot
GLOBAL_INCLUDE_PATH=-I ../redisLinux -I ./APIModules/QTSSAuthModule -I ./APIStubLib

# Link Libraries and Options
LDLIBSOPTIONS=-L../CommonUtilitiesLib/${CND_CONF} -L../EasyProtocol/EasyProtocol/${CND_CONF} -L../EasyProtocol/jsoncpp/${CND_CONF} -L../Lib/${CND_CONF}

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_CONF}/easycms

${CND_CONF}/easycms: ${OBJECTFILES}
	${MKDIR} -p ${CND_CONF}
	${LINK.cc} -o ${CND_CONF}/easycms ${OBJECTFILES} ${LDLIBSOPTIONS} -lCommonUtilitiesLib -ldl -lpthread -lEasyProtocol -ljsoncpp -lboost_system -lboost_thread -lrt -lhiredis

${OBJECTDIR}/_ext/b9fc5c32/HTTPProtocol.o: ../HTTPUtilitiesLib/HTTPProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/b9fc5c32
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b9fc5c32/HTTPProtocol.o ../HTTPUtilitiesLib/HTTPProtocol.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/_ext/b9fc5c32/HTTPRequest.o: ../HTTPUtilitiesLib/HTTPRequest.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/b9fc5c32
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b9fc5c32/HTTPRequest.o ../HTTPUtilitiesLib/HTTPRequest.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/_ext/dfc17a71/OSMemory.o: ../OSMemoryLib/OSMemory.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/dfc17a71
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/dfc17a71/OSMemory.o ../OSMemoryLib/OSMemory.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/_ext/cbf2331e/InternalStdLib.o: ../SafeStdLib/InternalStdLib.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/cbf2331e
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cbf2331e/InternalStdLib.o ../SafeStdLib/InternalStdLib.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/APICommonCode/QTAccessFile.o: APICommonCode/QTAccessFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTAccessFile.o APICommonCode/QTAccessFile.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o: APICommonCode/QTSSModuleUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o APICommonCode/QTSSModuleUtils.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/APICommonCode/QTSSRollingLog.o: APICommonCode/QTSSRollingLog.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTSSRollingLog.o APICommonCode/QTSSRollingLog.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/APIStubLib/QTSS_Private.o: APIStubLib/QTSS_Private.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIStubLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIStubLib/QTSS_Private.o APIStubLib/QTSS_Private.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o: PrefsSourceLib/FilePrefsSource.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o PrefsSourceLib/FilePrefsSource.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/PrefsSourceLib/XMLParser.o: PrefsSourceLib/XMLParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLParser.o PrefsSourceLib/XMLParser.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o: PrefsSourceLib/XMLPrefsParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o PrefsSourceLib/XMLPrefsParser.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o: Server.tproj/GenerateXMLPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o Server.tproj/GenerateXMLPrefs.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/HTTPRequestStream.o: Server.tproj/HTTPRequestStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/HTTPRequestStream.o Server.tproj/HTTPRequestStream.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/HTTPResponseStream.o: Server.tproj/HTTPResponseStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/HTTPResponseStream.o Server.tproj/HTTPResponseStream.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/HTTPSession.o: Server.tproj/HTTPSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/HTTPSession.o Server.tproj/HTTPSession.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/HTTPSessionInterface.o: Server.tproj/HTTPSessionInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/HTTPSessionInterface.o Server.tproj/HTTPSessionInterface.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSSCallbacks.o: Server.tproj/QTSSCallbacks.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSCallbacks.o Server.tproj/QTSSCallbacks.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSSDataConverter.o: Server.tproj/QTSSDataConverter.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSDataConverter.o Server.tproj/QTSSDataConverter.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSSDictionary.o: Server.tproj/QTSSDictionary.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSDictionary.o Server.tproj/QTSSDictionary.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSSErrorLogModule.o: Server.tproj/QTSSErrorLogModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSErrorLogModule.o Server.tproj/QTSSErrorLogModule.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o: Server.tproj/QTSSExpirationDate.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o Server.tproj/QTSSExpirationDate.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSSFile.o: Server.tproj/QTSSFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSFile.o Server.tproj/QTSSFile.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSSMessages.o: Server.tproj/QTSSMessages.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSMessages.o Server.tproj/QTSSMessages.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSSModule.o: Server.tproj/QTSSModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSModule.o Server.tproj/QTSSModule.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSSPrefs.o: Server.tproj/QTSSPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSPrefs.o Server.tproj/QTSSPrefs.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSSSocket.o: Server.tproj/QTSSSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSSocket.o Server.tproj/QTSSSocket.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSServer.o: Server.tproj/QTSServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServer.o Server.tproj/QTSServer.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSServerInterface.o: Server.tproj/QTSServerInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerInterface.o Server.tproj/QTSServerInterface.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSServerPrefs.o: Server.tproj/QTSServerPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerPrefs.o Server.tproj/QTSServerPrefs.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/RTSPProtocol.o: Server.tproj/RTSPProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTSPProtocol.o Server.tproj/RTSPProtocol.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/RunServer.o: Server.tproj/RunServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RunServer.o Server.tproj/RunServer.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/QTSSAuthModule.o: Server.tproj/QTSSAuthModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSAuthModule.o Server.tproj/QTSSAuthModule.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Server.tproj/main.o: Server.tproj/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/main.o Server.tproj/main.cpp  ${GLOBAL_INCLUDE_PATH}

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
