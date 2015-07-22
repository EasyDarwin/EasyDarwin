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
AS=as

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
	${OBJECTDIR}/AVSXmlUtil.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../../Bin/ARM/libavsxmlutil.a

../../Bin/ARM/libavsxmlutil.a: ${OBJECTFILES}
	${MKDIR} -p ../../Bin/ARM
	${RM} ../../Bin/ARM/libavsxmlutil.a
	${AR} -rv ../../Bin/ARM/libavsxmlutil.a ${OBJECTFILES} 
	$(RANLIB) ../../Bin/ARM/libavsxmlutil.a

${OBJECTDIR}/AVSXmlUtil.o: AVSXmlUtil.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DEASYDARWIN -I../../Include/EasyDSSProtocol -I../Include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AVSXmlUtil.o AVSXmlUtil.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ../../Bin/ARM/libavsxmlutil.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
