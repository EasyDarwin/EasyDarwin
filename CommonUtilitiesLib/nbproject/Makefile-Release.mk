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
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/cbf2331e/InternalStdLib.o \
	${OBJECTDIR}/ClientSocket.o \
	${OBJECTDIR}/ConfParser.o \
	${OBJECTDIR}/DateTranslator.o \
	${OBJECTDIR}/EventContext.o \
	${OBJECTDIR}/Format.o \
	${OBJECTDIR}/GetWord.o \
	${OBJECTDIR}/IdleTask.o \
	${OBJECTDIR}/MyAssert.o \
	${OBJECTDIR}/OS.o \
	${OBJECTDIR}/OSBufferPool.o \
	${OBJECTDIR}/OSCodeFragment.o \
	${OBJECTDIR}/OSCond.o \
	${OBJECTDIR}/OSFileSource.o \
	${OBJECTDIR}/OSHeap.o \
	${OBJECTDIR}/OSMapEx.o \
	${OBJECTDIR}/OSMutex.o \
	${OBJECTDIR}/OSMutexRW.o \
	${OBJECTDIR}/OSQueue.o \
	${OBJECTDIR}/OSRef.o \
	${OBJECTDIR}/OSRefTableEx.o \
	${OBJECTDIR}/OSThread.o \
	${OBJECTDIR}/QueryParamList.o \
	${OBJECTDIR}/ResizeableStringFormatter.o \
	${OBJECTDIR}/SDPUtils.o \
	${OBJECTDIR}/Socket.o \
	${OBJECTDIR}/SocketUtils.o \
	${OBJECTDIR}/StrPtrLen.o \
	${OBJECTDIR}/StringFormatter.o \
	${OBJECTDIR}/StringParser.o \
	${OBJECTDIR}/StringTranslator.o \
	${OBJECTDIR}/TCPListenerSocket.o \
	${OBJECTDIR}/TCPSocket.o \
	${OBJECTDIR}/Task.o \
	${OBJECTDIR}/TimeoutTask.o \
	${OBJECTDIR}/Trim.o \
	${OBJECTDIR}/UDPDemuxer.o \
	${OBJECTDIR}/UDPSocket.o \
	${OBJECTDIR}/UDPSocketPool.o \
	${OBJECTDIR}/UserAgentParser.o \
	${OBJECTDIR}/atomic.o \
	${OBJECTDIR}/base64.o \
	${OBJECTDIR}/easy_gettimeofday.o \
	${OBJECTDIR}/epollEvent.o \
	${OBJECTDIR}/ev.o \
	${OBJECTDIR}/getopt.o \
	${OBJECTDIR}/keyframecache.o \
	${OBJECTDIR}/md5.o \
	${OBJECTDIR}/md5digest.o \
	${OBJECTDIR}/sdpCache.o


# C Compiler Flags
CFLAGS=-m32 -fPIC

# CC Compiler Flags
CCFLAGS=-m32 -fPIC
CXXFLAGS=-m32 -fPIC

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_CONF}/libCommonUtilitiesLib.a

${CND_CONF}/libCommonUtilitiesLib.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_CONF}
	${RM} ${CND_CONF}/libCommonUtilitiesLib.a
	${AR} -rv ${CND_CONF}/libCommonUtilitiesLib.a ${OBJECTFILES} 
	$(RANLIB) ${CND_CONF}/libCommonUtilitiesLib.a

${OBJECTDIR}/_ext/cbf2331e/InternalStdLib.o: ../SafeStdLib/InternalStdLib.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/cbf2331e
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cbf2331e/InternalStdLib.o ../SafeStdLib/InternalStdLib.cpp

${OBJECTDIR}/ClientSocket.o: ClientSocket.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ClientSocket.o ClientSocket.cpp

${OBJECTDIR}/ConfParser.o: ConfParser.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ConfParser.o ConfParser.cpp

${OBJECTDIR}/DateTranslator.o: DateTranslator.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DateTranslator.o DateTranslator.cpp

${OBJECTDIR}/EventContext.o: EventContext.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EventContext.o EventContext.cpp

${OBJECTDIR}/Format.o: Format.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Format.o Format.cpp

${OBJECTDIR}/GetWord.o: GetWord.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GetWord.o GetWord.c

${OBJECTDIR}/IdleTask.o: IdleTask.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/IdleTask.o IdleTask.cpp

${OBJECTDIR}/MyAssert.o: MyAssert.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MyAssert.o MyAssert.cpp

${OBJECTDIR}/OS.o: OS.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OS.o OS.cpp

${OBJECTDIR}/OSBufferPool.o: OSBufferPool.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSBufferPool.o OSBufferPool.cpp

${OBJECTDIR}/OSCodeFragment.o: OSCodeFragment.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSCodeFragment.o OSCodeFragment.cpp

${OBJECTDIR}/OSCond.o: OSCond.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSCond.o OSCond.cpp

${OBJECTDIR}/OSFileSource.o: OSFileSource.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSFileSource.o OSFileSource.cpp

${OBJECTDIR}/OSHeap.o: OSHeap.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSHeap.o OSHeap.cpp

${OBJECTDIR}/OSMapEx.o: OSMapEx.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSMapEx.o OSMapEx.cpp

${OBJECTDIR}/OSMutex.o: OSMutex.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSMutex.o OSMutex.cpp

${OBJECTDIR}/OSMutexRW.o: OSMutexRW.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSMutexRW.o OSMutexRW.cpp

${OBJECTDIR}/OSQueue.o: OSQueue.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSQueue.o OSQueue.cpp

${OBJECTDIR}/OSRef.o: OSRef.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSRef.o OSRef.cpp

${OBJECTDIR}/OSRefTableEx.o: OSRefTableEx.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSRefTableEx.o OSRefTableEx.cpp

${OBJECTDIR}/OSThread.o: OSThread.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSThread.o OSThread.cpp

${OBJECTDIR}/QueryParamList.o: QueryParamList.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QueryParamList.o QueryParamList.cpp

${OBJECTDIR}/ResizeableStringFormatter.o: ResizeableStringFormatter.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ResizeableStringFormatter.o ResizeableStringFormatter.cpp

${OBJECTDIR}/SDPUtils.o: SDPUtils.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SDPUtils.o SDPUtils.cpp

${OBJECTDIR}/Socket.o: Socket.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Socket.o Socket.cpp

${OBJECTDIR}/SocketUtils.o: SocketUtils.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SocketUtils.o SocketUtils.cpp

${OBJECTDIR}/StrPtrLen.o: StrPtrLen.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/StrPtrLen.o StrPtrLen.cpp

${OBJECTDIR}/StringFormatter.o: StringFormatter.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/StringFormatter.o StringFormatter.cpp

${OBJECTDIR}/StringParser.o: StringParser.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/StringParser.o StringParser.cpp

${OBJECTDIR}/StringTranslator.o: StringTranslator.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/StringTranslator.o StringTranslator.cpp

${OBJECTDIR}/TCPListenerSocket.o: TCPListenerSocket.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TCPListenerSocket.o TCPListenerSocket.cpp

${OBJECTDIR}/TCPSocket.o: TCPSocket.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TCPSocket.o TCPSocket.cpp

${OBJECTDIR}/Task.o: Task.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Task.o Task.cpp

${OBJECTDIR}/TimeoutTask.o: TimeoutTask.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TimeoutTask.o TimeoutTask.cpp

${OBJECTDIR}/Trim.o: Trim.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Trim.o Trim.c

${OBJECTDIR}/UDPDemuxer.o: UDPDemuxer.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UDPDemuxer.o UDPDemuxer.cpp

${OBJECTDIR}/UDPSocket.o: UDPSocket.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UDPSocket.o UDPSocket.cpp

${OBJECTDIR}/UDPSocketPool.o: UDPSocketPool.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UDPSocketPool.o UDPSocketPool.cpp

${OBJECTDIR}/UserAgentParser.o: UserAgentParser.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UserAgentParser.o UserAgentParser.cpp

${OBJECTDIR}/atomic.o: atomic.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/atomic.o atomic.cpp

${OBJECTDIR}/base64.o: base64.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/base64.o base64.c

${OBJECTDIR}/easy_gettimeofday.o: easy_gettimeofday.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/easy_gettimeofday.o easy_gettimeofday.cpp

${OBJECTDIR}/epollEvent.o: epollEvent.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/epollEvent.o epollEvent.cpp

${OBJECTDIR}/ev.o: ev.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ev.o ev.cpp

${OBJECTDIR}/getopt.o: getopt.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/getopt.o getopt.c

${OBJECTDIR}/keyframecache.o: keyframecache.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/keyframecache.o keyframecache.cpp

${OBJECTDIR}/md5.o: md5.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/md5.o md5.c

${OBJECTDIR}/md5digest.o: md5digest.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/md5digest.o md5digest.cpp

${OBJECTDIR}/sdpCache.o: sdpCache.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -I../EasyProtocol/Include -I../RTSPUtilitiesLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/sdpCache.o sdpCache.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
