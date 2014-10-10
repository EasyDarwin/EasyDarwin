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
	${OBJECTDIR}/_ext/873319650/InternalStdLib.o \
	${OBJECTDIR}/QTAtom.o \
	${OBJECTDIR}/QTAtom_dref.o \
	${OBJECTDIR}/QTAtom_elst.o \
	${OBJECTDIR}/QTAtom_hinf.o \
	${OBJECTDIR}/QTAtom_mdhd.o \
	${OBJECTDIR}/QTAtom_mvhd.o \
	${OBJECTDIR}/QTAtom_stco.o \
	${OBJECTDIR}/QTAtom_stsc.o \
	${OBJECTDIR}/QTAtom_stsd.o \
	${OBJECTDIR}/QTAtom_stss.o \
	${OBJECTDIR}/QTAtom_stsz.o \
	${OBJECTDIR}/QTAtom_stts.o \
	${OBJECTDIR}/QTAtom_tkhd.o \
	${OBJECTDIR}/QTAtom_tref.o \
	${OBJECTDIR}/QTFile.o \
	${OBJECTDIR}/QTFile_FileControlBlock.o \
	${OBJECTDIR}/QTHintTrack.o \
	${OBJECTDIR}/QTRTPFile.o \
	${OBJECTDIR}/QTTrack.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../${CND_CONF}/libQTFileLib.a

../${CND_CONF}/libQTFileLib.a: ${OBJECTFILES}
	${MKDIR} -p ../${CND_CONF}
	${RM} ../${CND_CONF}/libQTFileLib.a
	${AR} -rv ../${CND_CONF}/libQTFileLib.a ${OBJECTFILES} 
	$(RANLIB) ../${CND_CONF}/libQTFileLib.a

${OBJECTDIR}/_ext/873319650/InternalStdLib.o: ../SafeStdLib/InternalStdLib.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/873319650
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/873319650/InternalStdLib.o ../SafeStdLib/InternalStdLib.cpp

${OBJECTDIR}/QTAtom.o: QTAtom.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom.o QTAtom.cpp

${OBJECTDIR}/QTAtom_dref.o: QTAtom_dref.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_dref.o QTAtom_dref.cpp

${OBJECTDIR}/QTAtom_elst.o: QTAtom_elst.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_elst.o QTAtom_elst.cpp

${OBJECTDIR}/QTAtom_hinf.o: QTAtom_hinf.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_hinf.o QTAtom_hinf.cpp

${OBJECTDIR}/QTAtom_mdhd.o: QTAtom_mdhd.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_mdhd.o QTAtom_mdhd.cpp

${OBJECTDIR}/QTAtom_mvhd.o: QTAtom_mvhd.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_mvhd.o QTAtom_mvhd.cpp

${OBJECTDIR}/QTAtom_stco.o: QTAtom_stco.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_stco.o QTAtom_stco.cpp

${OBJECTDIR}/QTAtom_stsc.o: QTAtom_stsc.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_stsc.o QTAtom_stsc.cpp

${OBJECTDIR}/QTAtom_stsd.o: QTAtom_stsd.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_stsd.o QTAtom_stsd.cpp

${OBJECTDIR}/QTAtom_stss.o: QTAtom_stss.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_stss.o QTAtom_stss.cpp

${OBJECTDIR}/QTAtom_stsz.o: QTAtom_stsz.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_stsz.o QTAtom_stsz.cpp

${OBJECTDIR}/QTAtom_stts.o: QTAtom_stts.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_stts.o QTAtom_stts.cpp

${OBJECTDIR}/QTAtom_tkhd.o: QTAtom_tkhd.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_tkhd.o QTAtom_tkhd.cpp

${OBJECTDIR}/QTAtom_tref.o: QTAtom_tref.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTAtom_tref.o QTAtom_tref.cpp

${OBJECTDIR}/QTFile.o: QTFile.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFile.o QTFile.cpp

${OBJECTDIR}/QTFile_FileControlBlock.o: QTFile_FileControlBlock.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTFile_FileControlBlock.o QTFile_FileControlBlock.cpp

${OBJECTDIR}/QTHintTrack.o: QTHintTrack.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTHintTrack.o QTHintTrack.cpp

${OBJECTDIR}/QTRTPFile.o: QTRTPFile.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTRTPFile.o QTRTPFile.cpp

${OBJECTDIR}/QTTrack.o: QTTrack.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDSS_USE_API_CALLBACKS -I. -I../RTPMetaInfoLib -I../RTPMetaInfoLib -I../APIStubLib -I../CommonUtilitiesLib -include ../PlatformHeader.h -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/QTTrack.o QTTrack.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ../${CND_CONF}/libQTFileLib.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
