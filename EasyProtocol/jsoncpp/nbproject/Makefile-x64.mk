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
include jsoncpp-Makefile.mk

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/lib_json/json_reader.o \
	${OBJECTDIR}/src/lib_json/json_value.o \
	${OBJECTDIR}/src/lib_json/json_writer.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_CONF}/libjsoncpp.a

${CND_CONF}/libjsoncpp.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_CONF}
	${RM} ${CND_CONF}/libjsoncpp.a
	${AR} -rv ${CND_CONF}/libjsoncpp.a ${OBJECTFILES} 
	$(RANLIB) ${CND_CONF}/libjsoncpp.a

${OBJECTDIR}/src/lib_json/json_reader.o: src/lib_json/json_reader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/lib_json
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lib_json/json_reader.o src/lib_json/json_reader.cpp

${OBJECTDIR}/src/lib_json/json_value.o: src/lib_json/json_value.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/lib_json
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lib_json/json_value.o src/lib_json/json_value.cpp

${OBJECTDIR}/src/lib_json/json_writer.o: src/lib_json/json_writer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/lib_json
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/lib_json/json_writer.o src/lib_json/json_writer.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_CONF}/libjsoncpp.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
