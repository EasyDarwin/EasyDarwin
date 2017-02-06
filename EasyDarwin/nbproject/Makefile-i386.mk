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
CND_CONF=i386
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/b9fc5c32/HTTPClientRequestStream.o \
	${OBJECTDIR}/_ext/b9fc5c32/HTTPClientResponseStream.o \
	${OBJECTDIR}/_ext/b9fc5c32/HTTPProtocol.o \
	${OBJECTDIR}/_ext/b9fc5c32/HTTPRequest.o \
	${OBJECTDIR}/_ext/a7d13d49/RTSPProtocol.o \
	${OBJECTDIR}/APICommonCode/QTAccessFile.o \
	${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o \
	${OBJECTDIR}/APICommonCode/QTSSRollingLog.o \
	${OBJECTDIR}/APICommonCode/SDPSourceInfo.o \
	${OBJECTDIR}/APICommonCode/SourceInfo.o \
	${OBJECTDIR}/APIModules/EasyCMSModule/EasyCMSModule.o \
	${OBJECTDIR}/APIModules/EasyCMSModule/EasyCMSSession.o \
	${OBJECTDIR}/APIModules/EasyHLSModule/EasyHLSModule.o \
	${OBJECTDIR}/APIModules/EasyHLSModule/EasyHLSSession.o \
	${OBJECTDIR}/APIModules/EasyRTMPModule/EasyRTMPModule.o \
	${OBJECTDIR}/APIModules/EasyRTMPModule/EasyRTMPSession.o \
	${OBJECTDIR}/APIModules/EasyRedisModule/EasyRedisModule.o \
	${OBJECTDIR}/APIModules/QTSSAccessLogModule/QTSSAccessLogModule.o \
	${OBJECTDIR}/APIModules/QTSSAccessModule/AccessChecker.o \
	${OBJECTDIR}/APIModules/QTSSAccessModule/QTSSAccessModule.o \
	${OBJECTDIR}/APIModules/QTSSAdminModule/AdminElementNode.o \
	${OBJECTDIR}/APIModules/QTSSAdminModule/AdminQuery.o \
	${OBJECTDIR}/APIModules/QTSSAdminModule/QTSSAdminModule.o \
	${OBJECTDIR}/APIModules/QTSSAdminModule/frozen.o \
	${OBJECTDIR}/APIModules/QTSSAdminModule/mongoose.o \
	${OBJECTDIR}/APIModules/QTSSFlowControlModule/QTSSFlowControlModule.o \
	${OBJECTDIR}/APIModules/QTSSPOSIXFileSysModule/QTSSPosixFileSysModule.o \
	${OBJECTDIR}/APIModules/QTSSReflectorModule/QTSSReflectorModule.o \
	${OBJECTDIR}/APIModules/QTSSReflectorModule/RCFSourceInfo.o \
	${OBJECTDIR}/APIModules/QTSSReflectorModule/RTPSessionOutput.o \
	${OBJECTDIR}/APIModules/QTSSReflectorModule/ReflectorSession.o \
	${OBJECTDIR}/APIModules/QTSSReflectorModule/ReflectorStream.o \
	${OBJECTDIR}/APIModules/QTSSReflectorModule/SequenceNumberMap.o \
	${OBJECTDIR}/APIModules/QTSSWebDebugModule/QTSSWebDebugModule.o \
	${OBJECTDIR}/APIStubLib/QTSS_Private.o \
	${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o \
	${OBJECTDIR}/PrefsSourceLib/XMLParser.o \
	${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o \
	${OBJECTDIR}/QTFileLib/QTAtom.o \
	${OBJECTDIR}/QTFileLib/QTAtom_dref.o \
	${OBJECTDIR}/QTFileLib/QTAtom_elst.o \
	${OBJECTDIR}/QTFileLib/QTAtom_hinf.o \
	${OBJECTDIR}/QTFileLib/QTAtom_mdhd.o \
	${OBJECTDIR}/QTFileLib/QTAtom_mvhd.o \
	${OBJECTDIR}/QTFileLib/QTAtom_stco.o \
	${OBJECTDIR}/QTFileLib/QTAtom_stsc.o \
	${OBJECTDIR}/QTFileLib/QTAtom_stsd.o \
	${OBJECTDIR}/QTFileLib/QTAtom_stss.o \
	${OBJECTDIR}/QTFileLib/QTAtom_stsz.o \
	${OBJECTDIR}/QTFileLib/QTAtom_stts.o \
	${OBJECTDIR}/QTFileLib/QTAtom_tkhd.o \
	${OBJECTDIR}/QTFileLib/QTAtom_tref.o \
	${OBJECTDIR}/QTFileLib/QTFile.o \
	${OBJECTDIR}/QTFileLib/QTFile_FileControlBlock.o \
	${OBJECTDIR}/QTFileLib/QTHintTrack.o \
	${OBJECTDIR}/QTFileLib/QTRTPFile.o \
	${OBJECTDIR}/QTFileLib/QTTrack.o \
	${OBJECTDIR}/RTCPUtilitiesLib/RTCPAPPNADUPacket.o \
	${OBJECTDIR}/RTCPUtilitiesLib/RTCPAPPPacket.o \
	${OBJECTDIR}/RTCPUtilitiesLib/RTCPAPPQTSSPacket.o \
	${OBJECTDIR}/RTCPUtilitiesLib/RTCPAckPacket.o \
	${OBJECTDIR}/RTCPUtilitiesLib/RTCPPacket.o \
	${OBJECTDIR}/RTCPUtilitiesLib/RTCPSRPacket.o \
	${OBJECTDIR}/RTPMetaInfoLib/RTPMetaInfoPacket.o \
	${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o \
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
	${OBJECTDIR}/Server.tproj/QTSSUserProfile.o \
	${OBJECTDIR}/Server.tproj/QTSServer.o \
	${OBJECTDIR}/Server.tproj/QTSServerInterface.o \
	${OBJECTDIR}/Server.tproj/QTSServerPrefs.o \
	${OBJECTDIR}/Server.tproj/RTCPTask.o \
	${OBJECTDIR}/Server.tproj/RTPBandwidthTracker.o \
	${OBJECTDIR}/Server.tproj/RTPOverbufferWindow.o \
	${OBJECTDIR}/Server.tproj/RTPPacketResender.o \
	${OBJECTDIR}/Server.tproj/RTPSession.o \
	${OBJECTDIR}/Server.tproj/RTPSessionInterface.o \
	${OBJECTDIR}/Server.tproj/RTPStream.o \
	${OBJECTDIR}/Server.tproj/RTSPRequest.o \
	${OBJECTDIR}/Server.tproj/RTSPRequestInterface.o \
	${OBJECTDIR}/Server.tproj/RTSPRequestStream.o \
	${OBJECTDIR}/Server.tproj/RTSPResponseStream.o \
	${OBJECTDIR}/Server.tproj/RTSPSession.o \
	${OBJECTDIR}/Server.tproj/RTSPSessionInterface.o \
	${OBJECTDIR}/Server.tproj/RunServer.o \
	${OBJECTDIR}/Server.tproj/main.o


# C Compiler Flags
CFLAGS=-m32 -pipe -Wall -Wno-format-y2k

# CC Compiler Flags
CCFLAGS=-m32 -pipe -Wall -Wno-format-y2k
CXXFLAGS=-m32 -pipe -Wall -Wno-format-y2k

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../CommonUtilitiesLib/${CND_CONF} -L../EasyProtocol/EasyProtocol/${CND_CONF} -L../EasyProtocol/jsoncpp/${CND_CONF} -L../EasyRedisClient/${CND_CONF} -LLib

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_CONF}/easydarwin

${CND_CONF}/easydarwin: ${OBJECTFILES}
	${MKDIR} -p ${CND_CONF}
	${LINK.cc} -o ${CND_CONF}/easydarwin ${OBJECTFILES} ${LDLIBSOPTIONS} -lCommonUtilitiesLib -lpthread -ldl -lstdc++ -lm -lcrypt -lEasyProtocol -ljsoncpp -leasyredisclient -leasyrtmp -leasyrtspclient -lEasyAACEncoder

${OBJECTDIR}/_ext/b9fc5c32/HTTPClientRequestStream.o: ../HTTPUtilitiesLib/HTTPClientRequestStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/b9fc5c32
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b9fc5c32/HTTPClientRequestStream.o ../HTTPUtilitiesLib/HTTPClientRequestStream.cpp

${OBJECTDIR}/_ext/b9fc5c32/HTTPClientResponseStream.o: ../HTTPUtilitiesLib/HTTPClientResponseStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/b9fc5c32
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b9fc5c32/HTTPClientResponseStream.o ../HTTPUtilitiesLib/HTTPClientResponseStream.cpp

${OBJECTDIR}/_ext/b9fc5c32/HTTPProtocol.o: ../HTTPUtilitiesLib/HTTPProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/b9fc5c32
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b9fc5c32/HTTPProtocol.o ../HTTPUtilitiesLib/HTTPProtocol.cpp

${OBJECTDIR}/_ext/b9fc5c32/HTTPRequest.o: ../HTTPUtilitiesLib/HTTPRequest.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/b9fc5c32
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b9fc5c32/HTTPRequest.o ../HTTPUtilitiesLib/HTTPRequest.cpp

${OBJECTDIR}/_ext/a7d13d49/RTSPProtocol.o: ../RTSPUtilitiesLib/RTSPProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/a7d13d49
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/a7d13d49/RTSPProtocol.o ../RTSPUtilitiesLib/RTSPProtocol.cpp

${OBJECTDIR}/APICommonCode/QTAccessFile.o: APICommonCode/QTAccessFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTAccessFile.o APICommonCode/QTAccessFile.cpp

${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o: APICommonCode/QTSSModuleUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o APICommonCode/QTSSModuleUtils.cpp

${OBJECTDIR}/APICommonCode/QTSSRollingLog.o: APICommonCode/QTSSRollingLog.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTSSRollingLog.o APICommonCode/QTSSRollingLog.cpp

${OBJECTDIR}/APICommonCode/SDPSourceInfo.o: APICommonCode/SDPSourceInfo.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/SDPSourceInfo.o APICommonCode/SDPSourceInfo.cpp

${OBJECTDIR}/APICommonCode/SourceInfo.o: APICommonCode/SourceInfo.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/SourceInfo.o APICommonCode/SourceInfo.cpp

${OBJECTDIR}/APIModules/EasyCMSModule/EasyCMSModule.o: APIModules/EasyCMSModule/EasyCMSModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/EasyCMSModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/EasyCMSModule/EasyCMSModule.o APIModules/EasyCMSModule/EasyCMSModule.cpp

${OBJECTDIR}/APIModules/EasyCMSModule/EasyCMSSession.o: APIModules/EasyCMSModule/EasyCMSSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/EasyCMSModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/EasyCMSModule/EasyCMSSession.o APIModules/EasyCMSModule/EasyCMSSession.cpp

${OBJECTDIR}/APIModules/EasyHLSModule/EasyHLSModule.o: APIModules/EasyHLSModule/EasyHLSModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/EasyHLSModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/EasyHLSModule/EasyHLSModule.o APIModules/EasyHLSModule/EasyHLSModule.cpp

${OBJECTDIR}/APIModules/EasyHLSModule/EasyHLSSession.o: APIModules/EasyHLSModule/EasyHLSSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/EasyHLSModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/EasyHLSModule/EasyHLSSession.o APIModules/EasyHLSModule/EasyHLSSession.cpp

${OBJECTDIR}/APIModules/EasyRTMPModule/EasyRTMPModule.o: APIModules/EasyRTMPModule/EasyRTMPModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/EasyRTMPModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/EasyRTMPModule/EasyRTMPModule.o APIModules/EasyRTMPModule/EasyRTMPModule.cpp

${OBJECTDIR}/APIModules/EasyRTMPModule/EasyRTMPSession.o: APIModules/EasyRTMPModule/EasyRTMPSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/EasyRTMPModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/EasyRTMPModule/EasyRTMPSession.o APIModules/EasyRTMPModule/EasyRTMPSession.cpp

${OBJECTDIR}/APIModules/EasyRedisModule/EasyRedisModule.o: APIModules/EasyRedisModule/EasyRedisModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/EasyRedisModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/EasyRedisModule/EasyRedisModule.o APIModules/EasyRedisModule/EasyRedisModule.cpp

${OBJECTDIR}/APIModules/QTSSAccessLogModule/QTSSAccessLogModule.o: APIModules/QTSSAccessLogModule/QTSSAccessLogModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSAccessLogModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSAccessLogModule/QTSSAccessLogModule.o APIModules/QTSSAccessLogModule/QTSSAccessLogModule.cpp

${OBJECTDIR}/APIModules/QTSSAccessModule/AccessChecker.o: APIModules/QTSSAccessModule/AccessChecker.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSAccessModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSAccessModule/AccessChecker.o APIModules/QTSSAccessModule/AccessChecker.cpp

${OBJECTDIR}/APIModules/QTSSAccessModule/QTSSAccessModule.o: APIModules/QTSSAccessModule/QTSSAccessModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSAccessModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSAccessModule/QTSSAccessModule.o APIModules/QTSSAccessModule/QTSSAccessModule.cpp

${OBJECTDIR}/APIModules/QTSSAdminModule/AdminElementNode.o: APIModules/QTSSAdminModule/AdminElementNode.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSAdminModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSAdminModule/AdminElementNode.o APIModules/QTSSAdminModule/AdminElementNode.cpp

${OBJECTDIR}/APIModules/QTSSAdminModule/AdminQuery.o: APIModules/QTSSAdminModule/AdminQuery.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSAdminModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSAdminModule/AdminQuery.o APIModules/QTSSAdminModule/AdminQuery.cpp

${OBJECTDIR}/APIModules/QTSSAdminModule/QTSSAdminModule.o: APIModules/QTSSAdminModule/QTSSAdminModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSAdminModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSAdminModule/QTSSAdminModule.o APIModules/QTSSAdminModule/QTSSAdminModule.cpp

${OBJECTDIR}/APIModules/QTSSAdminModule/frozen.o: APIModules/QTSSAdminModule/frozen.c 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSAdminModule
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSAdminModule/frozen.o APIModules/QTSSAdminModule/frozen.c

${OBJECTDIR}/APIModules/QTSSAdminModule/mongoose.o: APIModules/QTSSAdminModule/mongoose.c 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSAdminModule
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSAdminModule/mongoose.o APIModules/QTSSAdminModule/mongoose.c

${OBJECTDIR}/APIModules/QTSSFlowControlModule/QTSSFlowControlModule.o: APIModules/QTSSFlowControlModule/QTSSFlowControlModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSFlowControlModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSFlowControlModule/QTSSFlowControlModule.o APIModules/QTSSFlowControlModule/QTSSFlowControlModule.cpp

${OBJECTDIR}/APIModules/QTSSPOSIXFileSysModule/QTSSPosixFileSysModule.o: APIModules/QTSSPOSIXFileSysModule/QTSSPosixFileSysModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSPOSIXFileSysModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSPOSIXFileSysModule/QTSSPosixFileSysModule.o APIModules/QTSSPOSIXFileSysModule/QTSSPosixFileSysModule.cpp

${OBJECTDIR}/APIModules/QTSSReflectorModule/QTSSReflectorModule.o: APIModules/QTSSReflectorModule/QTSSReflectorModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSReflectorModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSReflectorModule/QTSSReflectorModule.o APIModules/QTSSReflectorModule/QTSSReflectorModule.cpp

${OBJECTDIR}/APIModules/QTSSReflectorModule/RCFSourceInfo.o: APIModules/QTSSReflectorModule/RCFSourceInfo.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSReflectorModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSReflectorModule/RCFSourceInfo.o APIModules/QTSSReflectorModule/RCFSourceInfo.cpp

${OBJECTDIR}/APIModules/QTSSReflectorModule/RTPSessionOutput.o: APIModules/QTSSReflectorModule/RTPSessionOutput.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSReflectorModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSReflectorModule/RTPSessionOutput.o APIModules/QTSSReflectorModule/RTPSessionOutput.cpp

${OBJECTDIR}/APIModules/QTSSReflectorModule/ReflectorSession.o: APIModules/QTSSReflectorModule/ReflectorSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSReflectorModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSReflectorModule/ReflectorSession.o APIModules/QTSSReflectorModule/ReflectorSession.cpp

${OBJECTDIR}/APIModules/QTSSReflectorModule/ReflectorStream.o: APIModules/QTSSReflectorModule/ReflectorStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSReflectorModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSReflectorModule/ReflectorStream.o APIModules/QTSSReflectorModule/ReflectorStream.cpp

${OBJECTDIR}/APIModules/QTSSReflectorModule/SequenceNumberMap.o: APIModules/QTSSReflectorModule/SequenceNumberMap.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSReflectorModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSReflectorModule/SequenceNumberMap.o APIModules/QTSSReflectorModule/SequenceNumberMap.cpp

${OBJECTDIR}/APIModules/QTSSWebDebugModule/QTSSWebDebugModule.o: APIModules/QTSSWebDebugModule/QTSSWebDebugModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIModules/QTSSWebDebugModule
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIModules/QTSSWebDebugModule/QTSSWebDebugModule.o APIModules/QTSSWebDebugModule/QTSSWebDebugModule.cpp

${OBJECTDIR}/APIStubLib/QTSS_Private.o: APIStubLib/QTSS_Private.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIStubLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIStubLib/QTSS_Private.o APIStubLib/QTSS_Private.cpp

${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o: PrefsSourceLib/FilePrefsSource.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o PrefsSourceLib/FilePrefsSource.cpp

${OBJECTDIR}/PrefsSourceLib/XMLParser.o: PrefsSourceLib/XMLParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLParser.o PrefsSourceLib/XMLParser.cpp

${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o: PrefsSourceLib/XMLPrefsParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o PrefsSourceLib/XMLPrefsParser.cpp

${OBJECTDIR}/QTFileLib/QTAtom.o: QTFileLib/QTAtom.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom.o QTFileLib/QTAtom.cpp

${OBJECTDIR}/QTFileLib/QTAtom_dref.o: QTFileLib/QTAtom_dref.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_dref.o QTFileLib/QTAtom_dref.cpp

${OBJECTDIR}/QTFileLib/QTAtom_elst.o: QTFileLib/QTAtom_elst.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_elst.o QTFileLib/QTAtom_elst.cpp

${OBJECTDIR}/QTFileLib/QTAtom_hinf.o: QTFileLib/QTAtom_hinf.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_hinf.o QTFileLib/QTAtom_hinf.cpp

${OBJECTDIR}/QTFileLib/QTAtom_mdhd.o: QTFileLib/QTAtom_mdhd.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_mdhd.o QTFileLib/QTAtom_mdhd.cpp

${OBJECTDIR}/QTFileLib/QTAtom_mvhd.o: QTFileLib/QTAtom_mvhd.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_mvhd.o QTFileLib/QTAtom_mvhd.cpp

${OBJECTDIR}/QTFileLib/QTAtom_stco.o: QTFileLib/QTAtom_stco.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_stco.o QTFileLib/QTAtom_stco.cpp

${OBJECTDIR}/QTFileLib/QTAtom_stsc.o: QTFileLib/QTAtom_stsc.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_stsc.o QTFileLib/QTAtom_stsc.cpp

${OBJECTDIR}/QTFileLib/QTAtom_stsd.o: QTFileLib/QTAtom_stsd.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_stsd.o QTFileLib/QTAtom_stsd.cpp

${OBJECTDIR}/QTFileLib/QTAtom_stss.o: QTFileLib/QTAtom_stss.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_stss.o QTFileLib/QTAtom_stss.cpp

${OBJECTDIR}/QTFileLib/QTAtom_stsz.o: QTFileLib/QTAtom_stsz.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_stsz.o QTFileLib/QTAtom_stsz.cpp

${OBJECTDIR}/QTFileLib/QTAtom_stts.o: QTFileLib/QTAtom_stts.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_stts.o QTFileLib/QTAtom_stts.cpp

${OBJECTDIR}/QTFileLib/QTAtom_tkhd.o: QTFileLib/QTAtom_tkhd.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_tkhd.o QTFileLib/QTAtom_tkhd.cpp

${OBJECTDIR}/QTFileLib/QTAtom_tref.o: QTFileLib/QTAtom_tref.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTAtom_tref.o QTFileLib/QTAtom_tref.cpp

${OBJECTDIR}/QTFileLib/QTFile.o: QTFileLib/QTFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTFile.o QTFileLib/QTFile.cpp

${OBJECTDIR}/QTFileLib/QTFile_FileControlBlock.o: QTFileLib/QTFile_FileControlBlock.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTFile_FileControlBlock.o QTFileLib/QTFile_FileControlBlock.cpp

${OBJECTDIR}/QTFileLib/QTHintTrack.o: QTFileLib/QTHintTrack.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTHintTrack.o QTFileLib/QTHintTrack.cpp

${OBJECTDIR}/QTFileLib/QTRTPFile.o: QTFileLib/QTRTPFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTRTPFile.o QTFileLib/QTRTPFile.cpp

${OBJECTDIR}/QTFileLib/QTTrack.o: QTFileLib/QTTrack.cpp 
	${MKDIR} -p ${OBJECTDIR}/QTFileLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFileLib/QTTrack.o QTFileLib/QTTrack.cpp

${OBJECTDIR}/RTCPUtilitiesLib/RTCPAPPNADUPacket.o: RTCPUtilitiesLib/RTCPAPPNADUPacket.cpp 
	${MKDIR} -p ${OBJECTDIR}/RTCPUtilitiesLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RTCPUtilitiesLib/RTCPAPPNADUPacket.o RTCPUtilitiesLib/RTCPAPPNADUPacket.cpp

${OBJECTDIR}/RTCPUtilitiesLib/RTCPAPPPacket.o: RTCPUtilitiesLib/RTCPAPPPacket.cpp 
	${MKDIR} -p ${OBJECTDIR}/RTCPUtilitiesLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RTCPUtilitiesLib/RTCPAPPPacket.o RTCPUtilitiesLib/RTCPAPPPacket.cpp

${OBJECTDIR}/RTCPUtilitiesLib/RTCPAPPQTSSPacket.o: RTCPUtilitiesLib/RTCPAPPQTSSPacket.cpp 
	${MKDIR} -p ${OBJECTDIR}/RTCPUtilitiesLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RTCPUtilitiesLib/RTCPAPPQTSSPacket.o RTCPUtilitiesLib/RTCPAPPQTSSPacket.cpp

${OBJECTDIR}/RTCPUtilitiesLib/RTCPAckPacket.o: RTCPUtilitiesLib/RTCPAckPacket.cpp 
	${MKDIR} -p ${OBJECTDIR}/RTCPUtilitiesLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RTCPUtilitiesLib/RTCPAckPacket.o RTCPUtilitiesLib/RTCPAckPacket.cpp

${OBJECTDIR}/RTCPUtilitiesLib/RTCPPacket.o: RTCPUtilitiesLib/RTCPPacket.cpp 
	${MKDIR} -p ${OBJECTDIR}/RTCPUtilitiesLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RTCPUtilitiesLib/RTCPPacket.o RTCPUtilitiesLib/RTCPPacket.cpp

${OBJECTDIR}/RTCPUtilitiesLib/RTCPSRPacket.o: RTCPUtilitiesLib/RTCPSRPacket.cpp 
	${MKDIR} -p ${OBJECTDIR}/RTCPUtilitiesLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RTCPUtilitiesLib/RTCPSRPacket.o RTCPUtilitiesLib/RTCPSRPacket.cpp

${OBJECTDIR}/RTPMetaInfoLib/RTPMetaInfoPacket.o: RTPMetaInfoLib/RTPMetaInfoPacket.cpp 
	${MKDIR} -p ${OBJECTDIR}/RTPMetaInfoLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RTPMetaInfoLib/RTPMetaInfoPacket.o RTPMetaInfoLib/RTPMetaInfoPacket.cpp

${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o: Server.tproj/GenerateXMLPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o Server.tproj/GenerateXMLPrefs.cpp

${OBJECTDIR}/Server.tproj/HTTPSession.o: Server.tproj/HTTPSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/HTTPSession.o Server.tproj/HTTPSession.cpp

${OBJECTDIR}/Server.tproj/HTTPSessionInterface.o: Server.tproj/HTTPSessionInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/HTTPSessionInterface.o Server.tproj/HTTPSessionInterface.cpp

${OBJECTDIR}/Server.tproj/QTSSCallbacks.o: Server.tproj/QTSSCallbacks.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSCallbacks.o Server.tproj/QTSSCallbacks.cpp

${OBJECTDIR}/Server.tproj/QTSSDataConverter.o: Server.tproj/QTSSDataConverter.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSDataConverter.o Server.tproj/QTSSDataConverter.cpp

${OBJECTDIR}/Server.tproj/QTSSDictionary.o: Server.tproj/QTSSDictionary.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSDictionary.o Server.tproj/QTSSDictionary.cpp

${OBJECTDIR}/Server.tproj/QTSSErrorLogModule.o: Server.tproj/QTSSErrorLogModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSErrorLogModule.o Server.tproj/QTSSErrorLogModule.cpp

${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o: Server.tproj/QTSSExpirationDate.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o Server.tproj/QTSSExpirationDate.cpp

${OBJECTDIR}/Server.tproj/QTSSFile.o: Server.tproj/QTSSFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSFile.o Server.tproj/QTSSFile.cpp

${OBJECTDIR}/Server.tproj/QTSSMessages.o: Server.tproj/QTSSMessages.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSMessages.o Server.tproj/QTSSMessages.cpp

${OBJECTDIR}/Server.tproj/QTSSModule.o: Server.tproj/QTSSModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSModule.o Server.tproj/QTSSModule.cpp

${OBJECTDIR}/Server.tproj/QTSSPrefs.o: Server.tproj/QTSSPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSPrefs.o Server.tproj/QTSSPrefs.cpp

${OBJECTDIR}/Server.tproj/QTSSSocket.o: Server.tproj/QTSSSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSSocket.o Server.tproj/QTSSSocket.cpp

${OBJECTDIR}/Server.tproj/QTSSUserProfile.o: Server.tproj/QTSSUserProfile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSUserProfile.o Server.tproj/QTSSUserProfile.cpp

${OBJECTDIR}/Server.tproj/QTSServer.o: Server.tproj/QTSServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServer.o Server.tproj/QTSServer.cpp

${OBJECTDIR}/Server.tproj/QTSServerInterface.o: Server.tproj/QTSServerInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerInterface.o Server.tproj/QTSServerInterface.cpp

${OBJECTDIR}/Server.tproj/QTSServerPrefs.o: Server.tproj/QTSServerPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerPrefs.o Server.tproj/QTSServerPrefs.cpp

${OBJECTDIR}/Server.tproj/RTCPTask.o: Server.tproj/RTCPTask.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTCPTask.o Server.tproj/RTCPTask.cpp

${OBJECTDIR}/Server.tproj/RTPBandwidthTracker.o: Server.tproj/RTPBandwidthTracker.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTPBandwidthTracker.o Server.tproj/RTPBandwidthTracker.cpp

${OBJECTDIR}/Server.tproj/RTPOverbufferWindow.o: Server.tproj/RTPOverbufferWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTPOverbufferWindow.o Server.tproj/RTPOverbufferWindow.cpp

${OBJECTDIR}/Server.tproj/RTPPacketResender.o: Server.tproj/RTPPacketResender.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTPPacketResender.o Server.tproj/RTPPacketResender.cpp

${OBJECTDIR}/Server.tproj/RTPSession.o: Server.tproj/RTPSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTPSession.o Server.tproj/RTPSession.cpp

${OBJECTDIR}/Server.tproj/RTPSessionInterface.o: Server.tproj/RTPSessionInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTPSessionInterface.o Server.tproj/RTPSessionInterface.cpp

${OBJECTDIR}/Server.tproj/RTPStream.o: Server.tproj/RTPStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTPStream.o Server.tproj/RTPStream.cpp

${OBJECTDIR}/Server.tproj/RTSPRequest.o: Server.tproj/RTSPRequest.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTSPRequest.o Server.tproj/RTSPRequest.cpp

${OBJECTDIR}/Server.tproj/RTSPRequestInterface.o: Server.tproj/RTSPRequestInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTSPRequestInterface.o Server.tproj/RTSPRequestInterface.cpp

${OBJECTDIR}/Server.tproj/RTSPRequestStream.o: Server.tproj/RTSPRequestStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTSPRequestStream.o Server.tproj/RTSPRequestStream.cpp

${OBJECTDIR}/Server.tproj/RTSPResponseStream.o: Server.tproj/RTSPResponseStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTSPResponseStream.o Server.tproj/RTSPResponseStream.cpp

${OBJECTDIR}/Server.tproj/RTSPSession.o: Server.tproj/RTSPSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTSPSession.o Server.tproj/RTSPSession.cpp

${OBJECTDIR}/Server.tproj/RTSPSessionInterface.o: Server.tproj/RTSPSessionInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTSPSessionInterface.o Server.tproj/RTSPSessionInterface.cpp

${OBJECTDIR}/Server.tproj/RunServer.o: Server.tproj/RunServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RunServer.o Server.tproj/RunServer.cpp

${OBJECTDIR}/Server.tproj/main.o: Server.tproj/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -D_REENTRANT -D__USE_POSIX -D__linux__ -I../HTTPUtilitiesLib -I../CommonUtilitiesLib -IServer.tproj -IQTFileLib/ -IRTPMetaInfoLib/ -IPrefsSourceLib/ -IAPIStubLib/ -IAPICommonCode/ -IRTCPUtilitiesLib/ -IRTSPClientLib/ -IAPIModules/QTSSFileModule/ -IAPIModules/QTSSHttpFileModule/ -IAPIModules/QTSSAccessModule/ -IAPIModules/QTSSAccessLogModule/ -IAPIModules/QTSSPOSIXFileSysModule -IAPIModules/QTSSAdminModule/ -IAPIModules/QTSSReflectorModule/ -IAPIModules/QTSSWebDebugModule/ -IAPIModules/QTSSFlowControlModule/ -IAPIModules/EasyHLSModule -IAPIModules/EasyRelayModule -IInclude -I. -I../Include -I../EasyProtocol/Include -I../EasyProtocol/jsoncpp/include -IAPIModules/EasyCMSModule -IAPIModules/EasyRedisModule -I../EasyRedisClient -I../RTSPUtilitiesLib -IAPIModules/EasyRTMPModule -IAPIModules/EasyHLSModule -include ../Include/PlatformHeader.h -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/main.o Server.tproj/main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_CONF}/easydarwin

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
