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
	${OBJECTDIR}/EasyDSSProtocol.o \
	${OBJECTDIR}/EasyDSSProtocolBase.o \
	${OBJECTDIR}/EasyDSSUtil.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${AVS_ROOT}/lib/libEasyDSSProtocol.a

${AVS_ROOT}/lib/libEasyDSSProtocol.a: ${OBJECTFILES}
	${MKDIR} -p ${AVS_ROOT}/lib
	${RM} ${AVS_ROOT}/lib/libEasyDSSProtocol.a
	${AR} -rv ${AVS_ROOT}/lib/libEasyDSSProtocol.a ${OBJECTFILES} 
	$(RANLIB) ${AVS_ROOT}/lib/libEasyDSSProtocol.a

${OBJECTDIR}/EasyDSSProtocol.o: EasyDSSProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Include/EasyDSSProtocol -I../Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EasyDSSProtocol.o EasyDSSProtocol.cpp

${OBJECTDIR}/EasyDSSProtocolBase.o: EasyDSSProtocolBase.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Include/EasyDSSProtocol -I../Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EasyDSSProtocolBase.o EasyDSSProtocolBase.cpp

${OBJECTDIR}/EasyDSSUtil.o: EasyDSSUtil.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../Include/EasyDSSProtocol -I../Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EasyDSSUtil.o EasyDSSUtil.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${AVS_ROOT}/lib/libEasyDSSProtocol.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
