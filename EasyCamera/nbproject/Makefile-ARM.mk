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
CC=arm-none-linux-gnueabi-gcc
CCC=arm-none-linux-gnueabi-g++
CXX=arm-none-linux-gnueabi-g++
FC=gfortran
AS=arm-none-linux-gnueabi-as

# Macros
CND_PLATFORM=GM8126-Linux-x86
CND_DLIB_EXT=so
CND_CONF=ARM
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/APICommonCode/QTAccessFile.o \
	${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o \
	${OBJECTDIR}/APICommonCode/QTSSRollingLog.o \
	${OBJECTDIR}/APIStubLib/QTSS_Private.o \
	${OBJECTDIR}/HTTPUtilitiesLib/HTTPProtocol.o \
	${OBJECTDIR}/HTTPUtilitiesLib/HTTPRequest.o \
	${OBJECTDIR}/OSMemoryLib/OSMemory.o \
	${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o \
	${OBJECTDIR}/PrefsSourceLib/XMLParser.o \
	${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o \
	${OBJECTDIR}/SafeStdLib/InternalStdLib.o \
	${OBJECTDIR}/Server.tproj/EasyMediaSource.o \
	${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o \
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
CFLAGS=-m32

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-LLib/arm/gm8126 -LCommonUtilitiesLib/${CND_CONF}

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk Bin/easycamera

Bin/easycamera: ${OBJECTFILES}
	${MKDIR} -p Bin
	${LINK.cc} -o Bin/easycamera ${OBJECTFILES} ${LDLIBSOPTIONS} -ldl -lCommonUtilitiesLib -lNetLib -leasypusher -lpthread

${OBJECTDIR}/APICommonCode/QTAccessFile.o: APICommonCode/QTAccessFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTAccessFile.o APICommonCode/QTAccessFile.cpp

${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o: APICommonCode/QTSSModuleUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o APICommonCode/QTSSModuleUtils.cpp

${OBJECTDIR}/APICommonCode/QTSSRollingLog.o: APICommonCode/QTSSRollingLog.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTSSRollingLog.o APICommonCode/QTSSRollingLog.cpp

${OBJECTDIR}/APIStubLib/QTSS_Private.o: APIStubLib/QTSS_Private.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIStubLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIStubLib/QTSS_Private.o APIStubLib/QTSS_Private.cpp

${OBJECTDIR}/HTTPUtilitiesLib/HTTPProtocol.o: HTTPUtilitiesLib/HTTPProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/HTTPUtilitiesLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/HTTPUtilitiesLib/HTTPProtocol.o HTTPUtilitiesLib/HTTPProtocol.cpp

${OBJECTDIR}/HTTPUtilitiesLib/HTTPRequest.o: HTTPUtilitiesLib/HTTPRequest.cpp 
	${MKDIR} -p ${OBJECTDIR}/HTTPUtilitiesLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/HTTPUtilitiesLib/HTTPRequest.o HTTPUtilitiesLib/HTTPRequest.cpp

${OBJECTDIR}/OSMemoryLib/OSMemory.o: OSMemoryLib/OSMemory.cpp 
	${MKDIR} -p ${OBJECTDIR}/OSMemoryLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSMemoryLib/OSMemory.o OSMemoryLib/OSMemory.cpp

${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o: PrefsSourceLib/FilePrefsSource.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o PrefsSourceLib/FilePrefsSource.cpp

${OBJECTDIR}/PrefsSourceLib/XMLParser.o: PrefsSourceLib/XMLParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLParser.o PrefsSourceLib/XMLParser.cpp

${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o: PrefsSourceLib/XMLPrefsParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o PrefsSourceLib/XMLPrefsParser.cpp

${OBJECTDIR}/SafeStdLib/InternalStdLib.o: SafeStdLib/InternalStdLib.cpp 
	${MKDIR} -p ${OBJECTDIR}/SafeStdLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SafeStdLib/InternalStdLib.o SafeStdLib/InternalStdLib.cpp

${OBJECTDIR}/Server.tproj/EasyMediaSource.o: Server.tproj/EasyMediaSource.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/EasyMediaSource.o Server.tproj/EasyMediaSource.cpp

${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o: Server.tproj/GenerateXMLPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o Server.tproj/GenerateXMLPrefs.cpp

${OBJECTDIR}/Server.tproj/QTSSCallbacks.o: Server.tproj/QTSSCallbacks.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSCallbacks.o Server.tproj/QTSSCallbacks.cpp

${OBJECTDIR}/Server.tproj/QTSSDataConverter.o: Server.tproj/QTSSDataConverter.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSDataConverter.o Server.tproj/QTSSDataConverter.cpp

${OBJECTDIR}/Server.tproj/QTSSDictionary.o: Server.tproj/QTSSDictionary.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSDictionary.o Server.tproj/QTSSDictionary.cpp

${OBJECTDIR}/Server.tproj/QTSSErrorLogModule.o: Server.tproj/QTSSErrorLogModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSErrorLogModule.o Server.tproj/QTSSErrorLogModule.cpp

${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o: Server.tproj/QTSSExpirationDate.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o Server.tproj/QTSSExpirationDate.cpp

${OBJECTDIR}/Server.tproj/QTSSFile.o: Server.tproj/QTSSFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSFile.o Server.tproj/QTSSFile.cpp

${OBJECTDIR}/Server.tproj/QTSSMessages.o: Server.tproj/QTSSMessages.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSMessages.o Server.tproj/QTSSMessages.cpp

${OBJECTDIR}/Server.tproj/QTSSModule.o: Server.tproj/QTSSModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSModule.o Server.tproj/QTSSModule.cpp

${OBJECTDIR}/Server.tproj/QTSSPrefs.o: Server.tproj/QTSSPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSPrefs.o Server.tproj/QTSSPrefs.cpp

${OBJECTDIR}/Server.tproj/QTSSSocket.o: Server.tproj/QTSSSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSSocket.o Server.tproj/QTSSSocket.cpp

${OBJECTDIR}/Server.tproj/QTSServer.o: Server.tproj/QTSServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServer.o Server.tproj/QTSServer.cpp

${OBJECTDIR}/Server.tproj/QTSServerInterface.o: Server.tproj/QTSServerInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerInterface.o Server.tproj/QTSServerInterface.cpp

${OBJECTDIR}/Server.tproj/QTSServerPrefs.o: Server.tproj/QTSServerPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerPrefs.o Server.tproj/QTSServerPrefs.cpp

${OBJECTDIR}/Server.tproj/RunServer.o: Server.tproj/RunServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RunServer.o Server.tproj/RunServer.cpp

${OBJECTDIR}/Server.tproj/main.o: Server.tproj/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DHI_OS_LINUX -IAPICommonCode -IAPIStubLib -ICommonUtilitiesLib -IHTTPUtilitiesLib -IInclude -IServer.tproj -IPrefsSourceLib -I../../EasyDarwin/CMS/Include/libEasyCMS -I. -include PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/main.o Server.tproj/main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} Bin/easycamera

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
