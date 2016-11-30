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
	${OBJECTDIR}/EasyProtocol.o \
	${OBJECTDIR}/EasyProtocolBase.o \
	${OBJECTDIR}/EasyUtil.o


# C Compiler Flags
CFLAGS=

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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_CONF}/libEasyProtocol.a

${CND_CONF}/libEasyProtocol.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_CONF}
	${RM} ${CND_CONF}/libEasyProtocol.a
	${AR} -rv ${CND_CONF}/libEasyProtocol.a ${OBJECTFILES} 
	$(RANLIB) ${CND_CONF}/libEasyProtocol.a

${OBJECTDIR}/EasyProtocol.o: EasyProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../jsoncpp/include -I../Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EasyProtocol.o EasyProtocol.cpp

${OBJECTDIR}/EasyProtocolBase.o: EasyProtocolBase.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../jsoncpp/include -I../Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EasyProtocolBase.o EasyProtocolBase.cpp

${OBJECTDIR}/EasyUtil.o: EasyUtil.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../jsoncpp/include -I../Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EasyUtil.o EasyUtil.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_CONF}/libEasyProtocol.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
