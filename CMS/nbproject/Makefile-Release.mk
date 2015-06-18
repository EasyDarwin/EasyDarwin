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
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/622336651/ConfParser.o \
	${OBJECTDIR}/_ext/622336651/DateTranslator.o \
	${OBJECTDIR}/_ext/622336651/EventContext.o \
	${OBJECTDIR}/_ext/622336651/GetWord.o \
	${OBJECTDIR}/_ext/622336651/IdleTask.o \
	${OBJECTDIR}/_ext/622336651/MakeDir.o \
	${OBJECTDIR}/_ext/622336651/MyAssert.o \
	${OBJECTDIR}/_ext/622336651/OS.o \
	${OBJECTDIR}/_ext/622336651/OSBufferPool.o \
	${OBJECTDIR}/_ext/622336651/OSCodeFragment.o \
	${OBJECTDIR}/_ext/622336651/OSCond.o \
	${OBJECTDIR}/_ext/622336651/OSFileSource.o \
	${OBJECTDIR}/_ext/622336651/OSHeap.o \
	${OBJECTDIR}/_ext/622336651/OSMutex.o \
	${OBJECTDIR}/_ext/622336651/OSMutexRW.o \
	${OBJECTDIR}/_ext/622336651/OSQueue.o \
	${OBJECTDIR}/_ext/622336651/OSRef.o \
	${OBJECTDIR}/_ext/622336651/OSThread.o \
	${OBJECTDIR}/_ext/622336651/QueryParamList.o \
	${OBJECTDIR}/_ext/622336651/ResizeableStringFormatter.o \
	${OBJECTDIR}/_ext/622336651/SDPUtils.o \
	${OBJECTDIR}/_ext/622336651/Socket.o \
	${OBJECTDIR}/_ext/622336651/SocketUtils.o \
	${OBJECTDIR}/_ext/622336651/StrPtrLen.o \
	${OBJECTDIR}/_ext/622336651/StringFormatter.o \
	${OBJECTDIR}/_ext/622336651/StringParser.o \
	${OBJECTDIR}/_ext/622336651/StringTranslator.o \
	${OBJECTDIR}/_ext/622336651/TCPListenerSocket.o \
	${OBJECTDIR}/_ext/622336651/TCPSocket.o \
	${OBJECTDIR}/_ext/622336651/Task.o \
	${OBJECTDIR}/_ext/622336651/TimeoutTask.o \
	${OBJECTDIR}/_ext/622336651/Trim.o \
	${OBJECTDIR}/_ext/622336651/UDPDemuxer.o \
	${OBJECTDIR}/_ext/622336651/UDPSocket.o \
	${OBJECTDIR}/_ext/622336651/UDPSocketPool.o \
	${OBJECTDIR}/_ext/622336651/UserAgentParser.o \
	${OBJECTDIR}/_ext/622336651/atomic.o \
	${OBJECTDIR}/_ext/622336651/base64.o \
	${OBJECTDIR}/_ext/622336651/daemon.o \
	${OBJECTDIR}/_ext/622336651/ev.o \
	${OBJECTDIR}/_ext/622336651/getopt.o \
	${OBJECTDIR}/_ext/622336651/md5.o \
	${OBJECTDIR}/_ext/622336651/md5digest.o \
	${OBJECTDIR}/_ext/1174643662/HTTPProtocol.o \
	${OBJECTDIR}/_ext/1174643662/HTTPRequest.o \
	${OBJECTDIR}/_ext/540968335/OSMemory.o \
	${OBJECTDIR}/_ext/873319650/InternalStdLib.o \
	${OBJECTDIR}/APICommonCode/QTAccessFile.o \
	${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o \
	${OBJECTDIR}/APICommonCode/QTSSRollingLog.o \
	${OBJECTDIR}/APIStubLib/QTSS_Private.o \
	${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o \
	${OBJECTDIR}/PrefsSourceLib/XMLParser.o \
	${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o \
	${OBJECTDIR}/Server.tproj/BaseRequestInterface.o \
	${OBJECTDIR}/Server.tproj/BaseRequestStream.o \
	${OBJECTDIR}/Server.tproj/BaseResponseStream.o \
	${OBJECTDIR}/Server.tproj/BaseSessionInterface.o \
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
	${OBJECTDIR}/Server.tproj/QTSSUserProfile.o \
	${OBJECTDIR}/Server.tproj/QTSServer.o \
	${OBJECTDIR}/Server.tproj/QTSServerInterface.o \
	${OBJECTDIR}/Server.tproj/QTSServerPrefs.o \
	${OBJECTDIR}/Server.tproj/RTSPProtocol.o \
	${OBJECTDIR}/Server.tproj/RunServer.o \
	${OBJECTDIR}/Server.tproj/ServiceSession.o \
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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_CONF}/cms

${CND_CONF}/cms: ${OBJECTFILES}
	${MKDIR} -p ${CND_CONF}
	${LINK.cc} -o ${CND_CONF}/cms ${OBJECTFILES} ${LDLIBSOPTIONS} -ldl -lpthread

${OBJECTDIR}/_ext/622336651/ConfParser.o: ../CommonUtilitiesLib/ConfParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/ConfParser.o ../CommonUtilitiesLib/ConfParser.cpp

${OBJECTDIR}/_ext/622336651/DateTranslator.o: ../CommonUtilitiesLib/DateTranslator.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/DateTranslator.o ../CommonUtilitiesLib/DateTranslator.cpp

${OBJECTDIR}/_ext/622336651/EventContext.o: ../CommonUtilitiesLib/EventContext.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/EventContext.o ../CommonUtilitiesLib/EventContext.cpp

${OBJECTDIR}/_ext/622336651/GetWord.o: ../CommonUtilitiesLib/GetWord.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/GetWord.o ../CommonUtilitiesLib/GetWord.c

${OBJECTDIR}/_ext/622336651/IdleTask.o: ../CommonUtilitiesLib/IdleTask.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/IdleTask.o ../CommonUtilitiesLib/IdleTask.cpp

${OBJECTDIR}/_ext/622336651/MakeDir.o: ../CommonUtilitiesLib/MakeDir.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/MakeDir.o ../CommonUtilitiesLib/MakeDir.c

${OBJECTDIR}/_ext/622336651/MyAssert.o: ../CommonUtilitiesLib/MyAssert.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/MyAssert.o ../CommonUtilitiesLib/MyAssert.cpp

${OBJECTDIR}/_ext/622336651/OS.o: ../CommonUtilitiesLib/OS.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/OS.o ../CommonUtilitiesLib/OS.cpp

${OBJECTDIR}/_ext/622336651/OSBufferPool.o: ../CommonUtilitiesLib/OSBufferPool.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/OSBufferPool.o ../CommonUtilitiesLib/OSBufferPool.cpp

${OBJECTDIR}/_ext/622336651/OSCodeFragment.o: ../CommonUtilitiesLib/OSCodeFragment.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/OSCodeFragment.o ../CommonUtilitiesLib/OSCodeFragment.cpp

${OBJECTDIR}/_ext/622336651/OSCond.o: ../CommonUtilitiesLib/OSCond.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/OSCond.o ../CommonUtilitiesLib/OSCond.cpp

${OBJECTDIR}/_ext/622336651/OSFileSource.o: ../CommonUtilitiesLib/OSFileSource.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/OSFileSource.o ../CommonUtilitiesLib/OSFileSource.cpp

${OBJECTDIR}/_ext/622336651/OSHeap.o: ../CommonUtilitiesLib/OSHeap.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/OSHeap.o ../CommonUtilitiesLib/OSHeap.cpp

${OBJECTDIR}/_ext/622336651/OSMutex.o: ../CommonUtilitiesLib/OSMutex.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/OSMutex.o ../CommonUtilitiesLib/OSMutex.cpp

${OBJECTDIR}/_ext/622336651/OSMutexRW.o: ../CommonUtilitiesLib/OSMutexRW.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/OSMutexRW.o ../CommonUtilitiesLib/OSMutexRW.cpp

${OBJECTDIR}/_ext/622336651/OSQueue.o: ../CommonUtilitiesLib/OSQueue.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/OSQueue.o ../CommonUtilitiesLib/OSQueue.cpp

${OBJECTDIR}/_ext/622336651/OSRef.o: ../CommonUtilitiesLib/OSRef.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/OSRef.o ../CommonUtilitiesLib/OSRef.cpp

${OBJECTDIR}/_ext/622336651/OSThread.o: ../CommonUtilitiesLib/OSThread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/OSThread.o ../CommonUtilitiesLib/OSThread.cpp

${OBJECTDIR}/_ext/622336651/QueryParamList.o: ../CommonUtilitiesLib/QueryParamList.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/QueryParamList.o ../CommonUtilitiesLib/QueryParamList.cpp

${OBJECTDIR}/_ext/622336651/ResizeableStringFormatter.o: ../CommonUtilitiesLib/ResizeableStringFormatter.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/ResizeableStringFormatter.o ../CommonUtilitiesLib/ResizeableStringFormatter.cpp

${OBJECTDIR}/_ext/622336651/SDPUtils.o: ../CommonUtilitiesLib/SDPUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/SDPUtils.o ../CommonUtilitiesLib/SDPUtils.cpp

${OBJECTDIR}/_ext/622336651/Socket.o: ../CommonUtilitiesLib/Socket.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/Socket.o ../CommonUtilitiesLib/Socket.cpp

${OBJECTDIR}/_ext/622336651/SocketUtils.o: ../CommonUtilitiesLib/SocketUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/SocketUtils.o ../CommonUtilitiesLib/SocketUtils.cpp

${OBJECTDIR}/_ext/622336651/StrPtrLen.o: ../CommonUtilitiesLib/StrPtrLen.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/StrPtrLen.o ../CommonUtilitiesLib/StrPtrLen.cpp

${OBJECTDIR}/_ext/622336651/StringFormatter.o: ../CommonUtilitiesLib/StringFormatter.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/StringFormatter.o ../CommonUtilitiesLib/StringFormatter.cpp

${OBJECTDIR}/_ext/622336651/StringParser.o: ../CommonUtilitiesLib/StringParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/StringParser.o ../CommonUtilitiesLib/StringParser.cpp

${OBJECTDIR}/_ext/622336651/StringTranslator.o: ../CommonUtilitiesLib/StringTranslator.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/StringTranslator.o ../CommonUtilitiesLib/StringTranslator.cpp

${OBJECTDIR}/_ext/622336651/TCPListenerSocket.o: ../CommonUtilitiesLib/TCPListenerSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/TCPListenerSocket.o ../CommonUtilitiesLib/TCPListenerSocket.cpp

${OBJECTDIR}/_ext/622336651/TCPSocket.o: ../CommonUtilitiesLib/TCPSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/TCPSocket.o ../CommonUtilitiesLib/TCPSocket.cpp

${OBJECTDIR}/_ext/622336651/Task.o: ../CommonUtilitiesLib/Task.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/Task.o ../CommonUtilitiesLib/Task.cpp

${OBJECTDIR}/_ext/622336651/TimeoutTask.o: ../CommonUtilitiesLib/TimeoutTask.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/TimeoutTask.o ../CommonUtilitiesLib/TimeoutTask.cpp

${OBJECTDIR}/_ext/622336651/Trim.o: ../CommonUtilitiesLib/Trim.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/Trim.o ../CommonUtilitiesLib/Trim.c

${OBJECTDIR}/_ext/622336651/UDPDemuxer.o: ../CommonUtilitiesLib/UDPDemuxer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/UDPDemuxer.o ../CommonUtilitiesLib/UDPDemuxer.cpp

${OBJECTDIR}/_ext/622336651/UDPSocket.o: ../CommonUtilitiesLib/UDPSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/UDPSocket.o ../CommonUtilitiesLib/UDPSocket.cpp

${OBJECTDIR}/_ext/622336651/UDPSocketPool.o: ../CommonUtilitiesLib/UDPSocketPool.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/UDPSocketPool.o ../CommonUtilitiesLib/UDPSocketPool.cpp

${OBJECTDIR}/_ext/622336651/UserAgentParser.o: ../CommonUtilitiesLib/UserAgentParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/UserAgentParser.o ../CommonUtilitiesLib/UserAgentParser.cpp

${OBJECTDIR}/_ext/622336651/atomic.o: ../CommonUtilitiesLib/atomic.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/atomic.o ../CommonUtilitiesLib/atomic.cpp

${OBJECTDIR}/_ext/622336651/base64.o: ../CommonUtilitiesLib/base64.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/base64.o ../CommonUtilitiesLib/base64.c

${OBJECTDIR}/_ext/622336651/daemon.o: ../CommonUtilitiesLib/daemon.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/daemon.o ../CommonUtilitiesLib/daemon.c

${OBJECTDIR}/_ext/622336651/ev.o: ../CommonUtilitiesLib/ev.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/ev.o ../CommonUtilitiesLib/ev.cpp

${OBJECTDIR}/_ext/622336651/getopt.o: ../CommonUtilitiesLib/getopt.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/getopt.o ../CommonUtilitiesLib/getopt.c

${OBJECTDIR}/_ext/622336651/md5.o: ../CommonUtilitiesLib/md5.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/md5.o ../CommonUtilitiesLib/md5.c

${OBJECTDIR}/_ext/622336651/md5digest.o: ../CommonUtilitiesLib/md5digest.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/622336651
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/622336651/md5digest.o ../CommonUtilitiesLib/md5digest.cpp

${OBJECTDIR}/_ext/1174643662/HTTPProtocol.o: ../HTTPUtilitiesLib/HTTPProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1174643662
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1174643662/HTTPProtocol.o ../HTTPUtilitiesLib/HTTPProtocol.cpp

${OBJECTDIR}/_ext/1174643662/HTTPRequest.o: ../HTTPUtilitiesLib/HTTPRequest.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1174643662
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1174643662/HTTPRequest.o ../HTTPUtilitiesLib/HTTPRequest.cpp

${OBJECTDIR}/_ext/540968335/OSMemory.o: ../OSMemoryLib/OSMemory.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/540968335
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/540968335/OSMemory.o ../OSMemoryLib/OSMemory.cpp

${OBJECTDIR}/_ext/873319650/InternalStdLib.o: ../SafeStdLib/InternalStdLib.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/873319650
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/873319650/InternalStdLib.o ../SafeStdLib/InternalStdLib.cpp

${OBJECTDIR}/APICommonCode/QTAccessFile.o: APICommonCode/QTAccessFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTAccessFile.o APICommonCode/QTAccessFile.cpp

${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o: APICommonCode/QTSSModuleUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTSSModuleUtils.o APICommonCode/QTSSModuleUtils.cpp

${OBJECTDIR}/APICommonCode/QTSSRollingLog.o: APICommonCode/QTSSRollingLog.cpp 
	${MKDIR} -p ${OBJECTDIR}/APICommonCode
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APICommonCode/QTSSRollingLog.o APICommonCode/QTSSRollingLog.cpp

${OBJECTDIR}/APIStubLib/QTSS_Private.o: APIStubLib/QTSS_Private.cpp 
	${MKDIR} -p ${OBJECTDIR}/APIStubLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/APIStubLib/QTSS_Private.o APIStubLib/QTSS_Private.cpp

${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o: PrefsSourceLib/FilePrefsSource.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/FilePrefsSource.o PrefsSourceLib/FilePrefsSource.cpp

${OBJECTDIR}/PrefsSourceLib/XMLParser.o: PrefsSourceLib/XMLParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLParser.o PrefsSourceLib/XMLParser.cpp

${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o: PrefsSourceLib/XMLPrefsParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/PrefsSourceLib
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrefsSourceLib/XMLPrefsParser.o PrefsSourceLib/XMLPrefsParser.cpp

${OBJECTDIR}/Server.tproj/BaseRequestInterface.o: Server.tproj/BaseRequestInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/BaseRequestInterface.o Server.tproj/BaseRequestInterface.cpp

${OBJECTDIR}/Server.tproj/BaseRequestStream.o: Server.tproj/BaseRequestStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/BaseRequestStream.o Server.tproj/BaseRequestStream.cpp

${OBJECTDIR}/Server.tproj/BaseResponseStream.o: Server.tproj/BaseResponseStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/BaseResponseStream.o Server.tproj/BaseResponseStream.cpp

${OBJECTDIR}/Server.tproj/BaseSessionInterface.o: Server.tproj/BaseSessionInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/BaseSessionInterface.o Server.tproj/BaseSessionInterface.cpp

${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o: Server.tproj/GenerateXMLPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/GenerateXMLPrefs.o Server.tproj/GenerateXMLPrefs.cpp

${OBJECTDIR}/Server.tproj/QTSSCallbacks.o: Server.tproj/QTSSCallbacks.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSCallbacks.o Server.tproj/QTSSCallbacks.cpp

${OBJECTDIR}/Server.tproj/QTSSDataConverter.o: Server.tproj/QTSSDataConverter.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSDataConverter.o Server.tproj/QTSSDataConverter.cpp

${OBJECTDIR}/Server.tproj/QTSSDictionary.o: Server.tproj/QTSSDictionary.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSDictionary.o Server.tproj/QTSSDictionary.cpp

${OBJECTDIR}/Server.tproj/QTSSErrorLogModule.o: Server.tproj/QTSSErrorLogModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSErrorLogModule.o Server.tproj/QTSSErrorLogModule.cpp

${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o: Server.tproj/QTSSExpirationDate.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSExpirationDate.o Server.tproj/QTSSExpirationDate.cpp

${OBJECTDIR}/Server.tproj/QTSSFile.o: Server.tproj/QTSSFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSFile.o Server.tproj/QTSSFile.cpp

${OBJECTDIR}/Server.tproj/QTSSMessages.o: Server.tproj/QTSSMessages.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSMessages.o Server.tproj/QTSSMessages.cpp

${OBJECTDIR}/Server.tproj/QTSSModule.o: Server.tproj/QTSSModule.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSModule.o Server.tproj/QTSSModule.cpp

${OBJECTDIR}/Server.tproj/QTSSPrefs.o: Server.tproj/QTSSPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSPrefs.o Server.tproj/QTSSPrefs.cpp

${OBJECTDIR}/Server.tproj/QTSSSocket.o: Server.tproj/QTSSSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSSocket.o Server.tproj/QTSSSocket.cpp

${OBJECTDIR}/Server.tproj/QTSSUserProfile.o: Server.tproj/QTSSUserProfile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSSUserProfile.o Server.tproj/QTSSUserProfile.cpp

${OBJECTDIR}/Server.tproj/QTSServer.o: Server.tproj/QTSServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServer.o Server.tproj/QTSServer.cpp

${OBJECTDIR}/Server.tproj/QTSServerInterface.o: Server.tproj/QTSServerInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerInterface.o Server.tproj/QTSServerInterface.cpp

${OBJECTDIR}/Server.tproj/QTSServerPrefs.o: Server.tproj/QTSServerPrefs.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/QTSServerPrefs.o Server.tproj/QTSServerPrefs.cpp

${OBJECTDIR}/Server.tproj/RTSPProtocol.o: Server.tproj/RTSPProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RTSPProtocol.o Server.tproj/RTSPProtocol.cpp

${OBJECTDIR}/Server.tproj/RunServer.o: Server.tproj/RunServer.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/RunServer.o Server.tproj/RunServer.cpp

${OBJECTDIR}/Server.tproj/ServiceSession.o: Server.tproj/ServiceSession.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/ServiceSession.o Server.tproj/ServiceSession.cpp

${OBJECTDIR}/Server.tproj/main.o: Server.tproj/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/Server.tproj
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IAPICommonCode -IAPIStubLib -IPrefsSourceLib -IServer.tproj -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../Include -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.tproj/main.o Server.tproj/main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_CONF}/cms

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
