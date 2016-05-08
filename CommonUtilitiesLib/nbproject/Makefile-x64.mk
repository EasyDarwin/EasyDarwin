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
	${OBJECTDIR}/_ext/dfc17a71/OSMemory.o \
	${OBJECTDIR}/_ext/cbf2331e/InternalStdLib.o \
	${OBJECTDIR}/ConfParser.o \
	${OBJECTDIR}/DateTranslator.o \
	${OBJECTDIR}/EventContext.o \
	${OBJECTDIR}/GetWord.o \
	${OBJECTDIR}/IdleTask.o \
	${OBJECTDIR}/MyAssert.o \
	${OBJECTDIR}/OS.o \
	${OBJECTDIR}/OSBufferPool.o \
	${OBJECTDIR}/OSCodeFragment.o \
	${OBJECTDIR}/OSCond.o \
	${OBJECTDIR}/OSFileSource.o \
	${OBJECTDIR}/OSHeap.o \
	${OBJECTDIR}/OSMutex.o \
	${OBJECTDIR}/OSMutexRW.o \
	${OBJECTDIR}/OSQueue.o \
	${OBJECTDIR}/OSRef.o \
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
	${OBJECTDIR}/epollEvent.o \
	${OBJECTDIR}/ev.o \
	${OBJECTDIR}/getopt.o \
	${OBJECTDIR}/keyframecache.o \
	${OBJECTDIR}/md5.o \
	${OBJECTDIR}/md5digest.o \
	${OBJECTDIR}/sdpCache.o\
	${OBJECTDIR}/OSMapEx.o\
	${OBJECTDIR}/OSRefTableEx.o

GLOBAL_INCLUDE_PATH=-I ../EasyCMS/APICommonCode  -I ../EasyCMS/APIStubLib
# C Compiler Flags
CFLAGS=-fPIC

# CC Compiler Flags
CCFLAGS=-fPIC
CXXFLAGS=-fPIC

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

${OBJECTDIR}/_ext/dfc17a71/OSMemory.o: ../OSMemoryLib/OSMemory.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/dfc17a71
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/dfc17a71/OSMemory.o ../OSMemoryLib/OSMemory.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/_ext/cbf2331e/InternalStdLib.o: ../SafeStdLib/InternalStdLib.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/cbf2331e
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/cbf2331e/InternalStdLib.o ../SafeStdLib/InternalStdLib.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/ConfParser.o: ConfParser.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ConfParser.o ConfParser.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/DateTranslator.o: DateTranslator.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DateTranslator.o DateTranslator.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/EventContext.o: EventContext.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EventContext.o EventContext.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/GetWord.o: GetWord.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GetWord.o GetWord.c ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/IdleTask.o: IdleTask.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/IdleTask.o IdleTask.cpp ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/MyAssert.o: MyAssert.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MyAssert.o MyAssert.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OS.o: OS.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OS.o OS.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OSBufferPool.o: OSBufferPool.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSBufferPool.o OSBufferPool.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OSCodeFragment.o: OSCodeFragment.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSCodeFragment.o OSCodeFragment.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OSCond.o: OSCond.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSCond.o OSCond.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OSFileSource.o: OSFileSource.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSFileSource.o OSFileSource.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OSHeap.o: OSHeap.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSHeap.o OSHeap.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OSMutex.o: OSMutex.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSMutex.o OSMutex.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OSMutexRW.o: OSMutexRW.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSMutexRW.o OSMutexRW.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OSQueue.o: OSQueue.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSQueue.o OSQueue.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OSRef.o: OSRef.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSRef.o OSRef.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OSThread.o: OSThread.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSThread.o OSThread.cpp  ${GLOBAL_INCLUDE_PATH} 

${OBJECTDIR}/QueryParamList.o: QueryParamList.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QueryParamList.o QueryParamList.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/ResizeableStringFormatter.o: ResizeableStringFormatter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ResizeableStringFormatter.o ResizeableStringFormatter.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/SDPUtils.o: SDPUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SDPUtils.o SDPUtils.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Socket.o: Socket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Socket.o Socket.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/SocketUtils.o: SocketUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SocketUtils.o SocketUtils.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/StrPtrLen.o: StrPtrLen.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/StrPtrLen.o StrPtrLen.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/StringFormatter.o: StringFormatter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/StringFormatter.o StringFormatter.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/StringParser.o: StringParser.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/StringParser.o StringParser.cpp  ${GLOBAL_INCLUDE_PATH}
 
${OBJECTDIR}/StringTranslator.o: StringTranslator.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/StringTranslator.o StringTranslator.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/TCPListenerSocket.o: TCPListenerSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TCPListenerSocket.o TCPListenerSocket.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/TCPSocket.o: TCPSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TCPSocket.o TCPSocket.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Task.o: Task.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Task.o Task.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/TimeoutTask.o: TimeoutTask.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TimeoutTask.o TimeoutTask.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/Trim.o: Trim.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Trim.o Trim.c  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/UDPDemuxer.o: UDPDemuxer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UDPDemuxer.o UDPDemuxer.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/UDPSocket.o: UDPSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UDPSocket.o UDPSocket.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/UDPSocketPool.o: UDPSocketPool.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UDPSocketPool.o UDPSocketPool.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/UserAgentParser.o: UserAgentParser.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UserAgentParser.o UserAgentParser.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/atomic.o: atomic.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/atomic.o atomic.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/base64.o: base64.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/base64.o base64.c  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/epollEvent.o: epollEvent.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/epollEvent.o epollEvent.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/ev.o: ev.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ev.o ev.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/getopt.o: getopt.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/getopt.o getopt.c  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/keyframecache.o: keyframecache.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/keyframecache.o keyframecache.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/md5.o: md5.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/md5.o md5.c  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/md5digest.o: md5digest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/md5digest.o md5digest.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/sdpCache.o: sdpCache.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/sdpCache.o sdpCache.cpp  ${GLOBAL_INCLUDE_PATH}

${OBJECTDIR}/OSMapEx.o: OSMapEx.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSMapEx.o OSMapEx.cpp  ${GLOBAL_INCLUDE_PATH}
	
${OBJECTDIR}/OSRefTableEx.o: OSRefTableEx.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DCOMMON_UTILITIES_LIB -D_REENTRANT -D__USE_POSIX -D__linux__ -I. -I../Include -I../EasyDarwin/APICommonCode -I../EasyDarwin/APIStubLib -I../EasyDarwin/RTPMetaInfoLib -include ../Include/PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSRefTableEx.o OSRefTableEx.cpp  ${GLOBAL_INCLUDE_PATH}

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_CONF}/libCommonUtilitiesLib.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
