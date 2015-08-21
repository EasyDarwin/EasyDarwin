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
	${OBJECTDIR}/_ext/1174643662/HTTPProtocol.o \
	${OBJECTDIR}/_ext/1174643662/HTTPRequest.o \
	${OBJECTDIR}/ClientSocket.o \
	${OBJECTDIR}/CommonDef.o \
	${OBJECTDIR}/EasyDarwinCMSAPI.o \
	${OBJECTDIR}/EasyDeviceCenter.o \
	${OBJECTDIR}/HttpClient.o \
	${OBJECTDIR}/OSMemory.o \
	${OBJECTDIR}/commonfunc.o \
	${OBJECTDIR}/libTinyDispatchEvent/TinyDispatchCenter.o \
	${OBJECTDIR}/libTinyDispatchEvent/libTinyDispatchCenter.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m32
CXXFLAGS=-m32

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../lib/Linux

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_CONF}/libEasyCMS.${CND_DLIB_EXT}

${CND_CONF}/libEasyCMS.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_CONF}
	${LINK.cc} -o ${CND_CONF}/libEasyCMS.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -lEasyProtocol -lCommonUtilitiesLib -shared -fPIC

${OBJECTDIR}/_ext/1174643662/HTTPProtocol.o: ../HTTPUtilitiesLib/HTTPProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1174643662
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHI_OS_LINUX -DUSE_POSIX -D_REENTRANT -D__linux__ -I../APIStubLib -IlibTinyDispatchEvent -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../include/EasyDSSBase -I../include/EasyProtocol -I../include -include PlatformHeader.h -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1174643662/HTTPProtocol.o ../HTTPUtilitiesLib/HTTPProtocol.cpp

${OBJECTDIR}/_ext/1174643662/HTTPRequest.o: ../HTTPUtilitiesLib/HTTPRequest.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1174643662
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHI_OS_LINUX -DUSE_POSIX -D_REENTRANT -D__linux__ -I../APIStubLib -IlibTinyDispatchEvent -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../include/EasyDSSBase -I../include/EasyProtocol -I../include -include PlatformHeader.h -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1174643662/HTTPRequest.o ../HTTPUtilitiesLib/HTTPRequest.cpp

${OBJECTDIR}/ClientSocket.o: ClientSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHI_OS_LINUX -DUSE_POSIX -D_REENTRANT -D__linux__ -I../APIStubLib -IlibTinyDispatchEvent -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../include/EasyDSSBase -I../include/EasyProtocol -I../include -include PlatformHeader.h -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ClientSocket.o ClientSocket.cpp

${OBJECTDIR}/CommonDef.o: CommonDef.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHI_OS_LINUX -DUSE_POSIX -D_REENTRANT -D__linux__ -I../APIStubLib -IlibTinyDispatchEvent -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../include/EasyDSSBase -I../include/EasyProtocol -I../include -include PlatformHeader.h -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CommonDef.o CommonDef.cpp

${OBJECTDIR}/EasyDarwinCMSAPI.o: EasyDarwinCMSAPI.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHI_OS_LINUX -DUSE_POSIX -D_REENTRANT -D__linux__ -I../APIStubLib -IlibTinyDispatchEvent -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../include/EasyDSSBase -I../include/EasyProtocol -I../include -include PlatformHeader.h -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EasyDarwinCMSAPI.o EasyDarwinCMSAPI.cpp

${OBJECTDIR}/EasyDeviceCenter.o: EasyDeviceCenter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHI_OS_LINUX -DUSE_POSIX -D_REENTRANT -D__linux__ -I../APIStubLib -IlibTinyDispatchEvent -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../include/EasyDSSBase -I../include/EasyProtocol -I../include -include PlatformHeader.h -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EasyDeviceCenter.o EasyDeviceCenter.cpp

${OBJECTDIR}/HttpClient.o: HttpClient.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHI_OS_LINUX -DUSE_POSIX -D_REENTRANT -D__linux__ -I../APIStubLib -IlibTinyDispatchEvent -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../include/EasyDSSBase -I../include/EasyProtocol -I../include -include PlatformHeader.h -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/HttpClient.o HttpClient.cpp

${OBJECTDIR}/OSMemory.o: OSMemory.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHI_OS_LINUX -DUSE_POSIX -D_REENTRANT -D__linux__ -I../APIStubLib -IlibTinyDispatchEvent -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../include/EasyDSSBase -I../include/EasyProtocol -I../include -include PlatformHeader.h -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OSMemory.o OSMemory.cpp

${OBJECTDIR}/commonfunc.o: commonfunc.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHI_OS_LINUX -DUSE_POSIX -D_REENTRANT -D__linux__ -I../APIStubLib -IlibTinyDispatchEvent -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../include/EasyDSSBase -I../include/EasyProtocol -I../include -include PlatformHeader.h -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/commonfunc.o commonfunc.cpp

${OBJECTDIR}/libTinyDispatchEvent/TinyDispatchCenter.o: libTinyDispatchEvent/TinyDispatchCenter.cpp 
	${MKDIR} -p ${OBJECTDIR}/libTinyDispatchEvent
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHI_OS_LINUX -DUSE_POSIX -D_REENTRANT -D__linux__ -I../APIStubLib -IlibTinyDispatchEvent -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../include/EasyDSSBase -I../include/EasyProtocol -I../include -include PlatformHeader.h -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libTinyDispatchEvent/TinyDispatchCenter.o libTinyDispatchEvent/TinyDispatchCenter.cpp

${OBJECTDIR}/libTinyDispatchEvent/libTinyDispatchCenter.o: libTinyDispatchEvent/libTinyDispatchCenter.cpp 
	${MKDIR} -p ${OBJECTDIR}/libTinyDispatchEvent
	${RM} "$@.d"
	$(COMPILE.cc) -g -DHI_OS_LINUX -DUSE_POSIX -D_REENTRANT -D__linux__ -I../APIStubLib -IlibTinyDispatchEvent -I../CommonUtilitiesLib -I../HTTPUtilitiesLib -I../include/EasyDSSBase -I../include/EasyProtocol -I../include -include PlatformHeader.h -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libTinyDispatchEvent/libTinyDispatchCenter.o libTinyDispatchEvent/libTinyDispatchCenter.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_CONF}/libEasyCMS.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
