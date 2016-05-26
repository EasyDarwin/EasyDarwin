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
	${OBJECTDIR}/AVSRedisClient.o \
	${OBJECTDIR}/async.o \
	${OBJECTDIR}/dict.o \
	${OBJECTDIR}/hiredis.o \
	${OBJECTDIR}/net.o \
	${OBJECTDIR}/read.o \
	${OBJECTDIR}/sds.o


# C Compiler Flags
CFLAGS=-std=c99

# CC Compiler Flags
CCFLAGS=-std=c++0x
CXXFLAGS=-std=c++0x

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${AVS_ROOT}/lib/libavsredisclient.a

${AVS_ROOT}/lib/libavsredisclient.a: ${OBJECTFILES}
	${MKDIR} -p ${AVS_ROOT}/lib
	${RM} ${AVS_ROOT}/lib/libavsredisclient.a
	${AR} -rv ${AVS_ROOT}/lib/libavsredisclient.a ${OBJECTFILES} 
	$(RANLIB) ${AVS_ROOT}/lib/libavsredisclient.a

${OBJECTDIR}/AVSRedisClient.o: AVSRedisClient.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I${AVS_ROOT}/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AVSRedisClient.o AVSRedisClient.cpp

${OBJECTDIR}/async.o: async.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/async.o async.c

${OBJECTDIR}/dict.o: dict.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dict.o dict.c

${OBJECTDIR}/hiredis.o: hiredis.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/hiredis.o hiredis.c

${OBJECTDIR}/net.o: net.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/net.o net.c

${OBJECTDIR}/read.o: read.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/read.o read.c

${OBJECTDIR}/sds.o: sds.c 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/sds.o sds.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${AVS_ROOT}/lib/libavsredisclient.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
