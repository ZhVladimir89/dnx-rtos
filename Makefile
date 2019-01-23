# Makefile for GNU make
####################################################################################################
#
# AUTHOR: DANIEL ZORYCHTA
#
# Version: 20130516
#--------------------------------------------------------------------------------------------------
#
#    Copyright (C) 2011, 2012, 2013  Daniel Zorychta (daniel.zorychta@gmail.com)
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the  Free Software  Foundation;  either version 2 of the License, or
#    any later version.
#
#    This  program  is  distributed  in the hope that  it will be useful,
#    but  WITHOUT  ANY  WARRANTY;  without  even  the implied warranty of
#    MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
#    GNU General Public License for more details.
#
#    You  should  have received a copy  of the GNU General Public License
#    along  with  this  program;  if not,  write  to  the  Free  Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#
####################################################################################################

include ./config/config.h

####################################################################################################
# PROJECT CONFIGURATION
####################################################################################################
# project name
PROJECT = $(__PROJECT_NAME__)

#---------------------------------------------------------------------------------------------------
# DEFAULT COMPILER FLAGS
#---------------------------------------------------------------------------------------------------
TOOLCHAIN = $(__PROJECT_TOOLCHAIN__)

AFLAGS   = -c \
           -g \
           -ggdb3 \
           -include ./config/config.h \
           -include ./build/defs.h \
           $(CPUCONFIG_AFLAGS)

CFLAGS   = -c \
           -g \
           -ggdb3  \
           -Os \
           -std=c99 \
           -ffunction-sections \
           -Wall \
           -Wextra \
           -Wparentheses \
           -Werror=implicit-function-declaration \
           -include ./config/config.h \
           -include ./build/defs.h \
           -DCOMPILE_EPOCH_TIME=$(shell $(DATE) "+%s") \
           $(CPUCONFIG_CFLAGS)

CXXFLAGS = -c \
           -g \
           -ggdb3 \
           -Os \
           -std=c++11 \
           -ffunction-sections \
           -fno-rtti \
           -fno-exceptions \
           -fno-unwind-tables \
           -Wall \
           -Wextra \
           -Wparentheses \
           -Werror=implicit-function-declaration \
           -include ./config/config.h \
           -DCOMPILE_EPOCH_TIME=$(shell $(DATE) "+%s") \
           -include ./build/defs.h \
           $(CPUCONFIG_CXXFLAGS)

LFLAGS   = -g \
           $(CPUCONFIG_LDFLAGS) \
           -Wl,--gc-sections \
           -Wl,-Map=$(TARGET_DIR_NAME)/$(TARGET)/$(PROJECT).map,--cref,--no-warn-mismatch \
           -Wall \
           -specs=nano.specs -specs=rdimon.specs \
           -lm

#---------------------------------------------------------------------------------------------------
# FILE EXTENSIONS CONFIGURATION
#---------------------------------------------------------------------------------------------------
OBJ_EXT = o
C_EXT   = c
CXX_EXT = cpp
AS_EXT  = s

#---------------------------------------------------------------------------------------------------
# FILE AND DIRECTORY NAMES
#---------------------------------------------------------------------------------------------------
# defines project path with binaries
TARGET_DIR_NAME = build

# defines object folder name
OBJ_DIR_NAME    = obj

# dependencies file name
DEP_FILE_NAME   = $(PROJECT).d

# folder localizations
APP_LOC         = src/application
APP_PRG_LOC     = $(APP_LOC)/programs
APP_LIB_LOC     = $(APP_LOC)/libs
SYS_LOC         = src/system

SYS_DRV_LOC     = $(SYS_LOC)/drivers
SYS_DRV_INC_LOC = $(SYS_LOC)/include/drivers
SYS_FS_LOC      = $(SYS_LOC)/fs
SYS_INIT_LOC    = $(SYS_LOC)/init
SYS_KRN_LOC     = $(SYS_LOC)/kernel
SYS_LIB_LOC     = $(SYS_LOC)/lib
SYS_MM_LOC      = $(SYS_LOC)/mm
SYS_NET_LOC     = $(SYS_LOC)/net
SYS_CPU_LOC     = $(SYS_LOC)/cpu
SYS_LIBC_LOC    = $(SYS_LOC)/libc

#---------------------------------------------------------------------------------------------------
# BASIC PROGRAMS DEFINITIONS
#---------------------------------------------------------------------------------------------------
SHELL      = /bin/bash
ECHO       = /bin/echo -e
RM         = /bin/rm -f
MKDIR      = /bin/mkdir -p
DATE       = /bin/date
CAT        = /bin/cat
GREP       = /bin/grep
UNAME      = /bin/uname -s
SIZEOF     = /usr/bin/stat -c %s
MKDEP      = /usr/bin/makedepend
WC         = /usr/bin/wc
CC         = $(TOOLCHAIN)gcc
CXX        = $(TOOLCHAIN)g++
LD         = $(TOOLCHAIN)g++
AS         = $(TOOLCHAIN)gcc -x assembler-with-cpp
OBJCOPY    = $(TOOLCHAIN)objcopy
OBJDUMP    = $(TOOLCHAIN)objdump
SIZE       = $(TOOLCHAIN)size
CONFIGTOOL = ./tools/configtool.sh
CODECHECK  = cppcheck
ADDAPPS    = ./$(APP_LOC)/addapps.sh
ADDFS      = ./$(SYS_FS_LOC)/addfs.sh
ADDDRIVERS = ./$(SYS_DRV_LOC)/adddriver.sh
FLASH_CPU  = ./tools/flash.sh
RESET_CPU  = ./tools/reset.sh
GIT_HOOKS  = ./tools/apply_git_hooks.sh
DOXYGEN    = ./tools/doxygen.sh
RELEASEPKG = ./tools/releasepkg.sh
RUNGENS    = ./tools/rungens.sh
FINDGVAR   = ./tools/find_global_vars.sh

#---------------------------------------------------------------------------------------------------
# MAKEFILE CORE (do not edit)
#---------------------------------------------------------------------------------------------------
# defines VALUES
EMPTY =

# defines this makefile name
THIS_MAKEFILE = $(firstword $(MAKEFILE_LIST))

# number of threads used in compilation (cpu count)
THREAD = $(shell $(ECHO) $$($(CAT) /proc/cpuinfo | $(GREP) processor | $(WC) -l))

# sets header search path (adds -I flags to paths)
SEARCHPATH          = $(foreach var, $(HDRLOC),-I$(var)) $(foreach var, $(HDRLOC_$(TARGET)),-I$(var))
SEARCHPATH_TARGET   = $(foreach var, $(HDRLOC_$(TARGET)),-I$(var)) -Isrc/
SEARCHPATH_PROGRAMS = $(foreach var, $(_HDRLOC_PROGRAMS),-I$(var))
SEARCHPATH_LIB      = $(foreach var, $(_HDRLOC_LIB),-I$(var))
SEARCHPATH_CORE     = $(foreach var, $(_HDRLOC_CORE),-I$(var))
SEARCHPATH_NOARCH   = $(foreach var, $(_HDRLOC_NOARCH),-I$(var))
SEARCHPATH_ARCH     = $(foreach var, $(_HDRLOC_ARCH),-I$(var))

# main target without defined prefixes
TARGET = $(__CPU_ARCH__)

# target path
TARGET_PATH = $(TARGET_DIR_NAME)/$(TARGET)

# object path
OBJ_PATH = $(TARGET_DIR_NAME)/$(TARGET)/$(OBJ_DIR_NAME)

# list of sources to compile
-include $(APP_LOC)/Makefile   # file is created in the addapps script
include $(SYS_LOC)/Makefile

# defines objects localizations
_HDRLOC_PROGRAMS = $(foreach file, $(HDRLOC_PROGRAMS),$(APP_PRG_LOC)/$(file))
_HDRLOC_LIB      = $(foreach file, $(HDRLOC_LIB),$(APP_LIB_LOC)/$(file))
_HDRLOC_CORE     = $(foreach file, $(HDRLOC_CORE),$(SYS_LOC)/$(file))
_HDRLOC_NOARCH   = $(foreach file, $(HDRLOC_NOARCH),$(SYS_LOC)/$(file))
_HDRLOC_ARCH     = $(foreach file, $(HDRLOC_ARCH),$(SYS_LOC)/$(file))
HDRLOC           = src/ $(_HDRLOC_PROGRAMS) $(_HDRLOC_LIB) $(_HDRLOC_CORE) $(_HDRLOC_NOARCH) $(_HDRLOC_ARCH)

# defines all C sources
CSRC    = $(foreach file, $(CSRC_PROGRAMS),$(APP_PRG_LOC)/$(file)) \
          $(foreach file, $(CSRC_LIB),$(APP_LIB_LOC)/$(file)) \
          $(foreach file, $(CSRC_CORE),$(SYS_LOC)/$(file)) \
          $(foreach file, $(CSRC_NOARCH),$(SYS_LOC)/$(file)) \
          $(foreach file, $(CSRC_ARCH),$(SYS_LOC)/$(file))

# defines all C++ sources
CXXSRC  = $(foreach file, $(CXXSRC_PROGRAMS),$(APP_PRG_LOC)/$(file)) \
          $(foreach file, $(CXXSRC_LIB),$(APP_LIB_LOC)/$(file)) \
          $(foreach file, $(CXXSRC_CORE),$(SYS_LOC)/$(file)) \
          $(foreach file, $(CXXSRC_NOARCH),$(SYS_LOC)/$(file)) \
          $(foreach file, $(CXXSRC_ARCH),$(SYS_LOC)/$(file))

# defines all assembler sources
ASRC    = $(foreach file, $(ASRC_ARCH),$(SYS_LOC)/$(file))

# defines objects names
OBJECTS = $(ASRC:.$(AS_EXT)=.$(OBJ_EXT)) $(CSRC:.$(C_EXT)=.$(OBJ_EXT)) $(CXXSRC:.$(CXX_EXT)=.$(OBJ_EXT))

# functions
FIND_GLOBAL_VARS_LIBS_PROGS = if [[ "$@" =~ $(APP_PRG_LOC) ]] || [[ "$@" =~ $(APP_LIB_LOC) ]]; then $(FINDGVAR) $@ || ($(RM) $@; exit 1); fi

####################################################################################################
# targets
####################################################################################################
.PHONY : all
all : generate apply_git_hooks
	@$(MAKE) -s -j 1 -f$(THIS_MAKEFILE) rungens
	@$(MAKE) -s -j 1 -f$(THIS_MAKEFILE) build_start

.PHONY : build_start
build_start : dependencies buildobjects linkobjects hex status

####################################################################################################
# help
####################################################################################################
.PHONY : help
help :
	@$(ECHO) "This is help for this $(THIS_MAKEFILE)"
	@$(ECHO) "Possible targets:"
	@$(ECHO) "   help                this help"
	@$(ECHO) "   config              project configuration (text mode)"
	@$(ECHO) "   clean               clean project"
	@$(ECHO) ""
	@$(ECHO) "Non-build targets:"
	@$(ECHO) "   check               static code analyze by using cppcheck"
	@$(ECHO) "   quickcheck          quick static code analyze by using cppcheck"
	@$(ECHO) "   flash, install      flash target CPU by using ./tools/flash.sh script"
	@$(ECHO) "   reset               reset target CPU by using ./tools/reset.sh script"
	@$(ECHO) "   release             create Release package"
	@$(ECHO) "   doc                 create documentation (Doxygen)"
	@$(ECHO) "   rungens             start generator scripts"

####################################################################################################
# project configuration wizard
####################################################################################################
.PHONY : config
config : clean apply_git_hooks
	@$(ECHO) "Starting configtool..."
	@$(CONFIGTOOL)

####################################################################################################
# analisis
####################################################################################################
.PHONY : check checkprog quickcheck appsyntaxcheck
check :
	@$(CODECHECK) -j $(THREAD) -q --std=c99 --std=c++11 \
	              --enable=warning,style,performance,portability,information,missingInclude \
	              --force --inconclusive \
	              --include=./config/project/flags.h \
	              $(SEARCHPATH) $(CSRC) $(CXXSRC)

checkprog :
	@$(CODECHECK) -j $(THREAD) -q --std=c99 --std=c++11 --enable=warning,style,missingInclude \
	              --inconclusive \
	              -DCOMPILE_EPOCH_TIME=0 \
	              $(SEARCHPATH)\
	              $(foreach file, $(CSRC_PROGRAMS),$(APP_PRG_LOC)/$(file))

quickcheck :
	@$(CODECHECK) -j $(THREAD) -q --std=c99 --std=c++11 \
	              --enable=warning,style,performance,portability,missingInclude \
	              --force --inconclusive \
	              --include=./config/project/flags.h \
	              -I src/system/include/libc/dnx \
	              $(CSRC) $(CXXSRC)

####################################################################################################
# create basic output files like hex, bin, lst etc.
####################################################################################################
.PHONY : hex
hex :
	@$(ECHO) 'Creating IHEX image...'
	@$(OBJCOPY) $(TARGET_PATH)/$(PROJECT).elf -O ihex $(TARGET_PATH)/$(PROJECT).hex

	@$(ECHO) 'Creating binary image...'
	@$(OBJCOPY) $(TARGET_PATH)/$(PROJECT).elf -O binary $(TARGET_PATH)/$(PROJECT).bin

	@$(ECHO) 'Creating memory dump...'
	@$(OBJDUMP) -x --syms $(TARGET_PATH)/$(PROJECT).elf > $(TARGET_PATH)/$(PROJECT).dmp

	@$(ECHO) 'Creating extended listing....'
	@$(OBJDUMP) -S $(TARGET_PATH)/$(PROJECT).elf > $(TARGET_PATH)/$(PROJECT).lst

	@$(ECHO) 'Creating objects size list...'
	@$(SIZE) -B -t --common $(foreach var,$(OBJECTS),$(OBJ_PATH)/$(var)) > $(TARGET_PATH)/$(PROJECT).size

	@$(ECHO) "Flash image size: $$($(SIZEOF) $(TARGET_PATH)/$(PROJECT).bin) bytes\n"

####################################################################################################
# show compile status
####################################################################################################
.PHONY : status
status :
	@$(ECHO) "-----------------------------------"
	@$(ECHO) "| `$(DATE) "+Compilation completed: %H:%M:%S"` |"
	@$(ECHO) "-----------------------------------"

####################################################################################################
####################################################################################################
# Adds programs and libraries to the project
# This target is used to generate ./src/programs/program_registration.c,
# ./src/programs/Makefile, and ./src/lib/Makefile files required in the
# build process
####################################################################################################
.PHONY : generate
generate :
	@$(MKDIR) $(TARGET_PATH)

	@$(ECHO) "Adding user's programs and libraries to the project..."
	@$(SHELL) $(ADDAPPS) ./$(APP_LOC)

	@$(ECHO) "Adding file systems to the project..."
	@$(SHELL) $(ADDFS) ./$(SYS_FS_LOC)

	@$(ECHO) "Adding drivers to the project..."
	@$(SHELL) $(ADDDRIVERS) ./$(SYS_DRV_LOC) ./$(SYS_DRV_INC_LOC)

	@$(ECHO) "Obtaining git hash..."
	@$(ECHO) "#ifndef COMMIT_HASH" > build/defs.h
	@$(ECHO) "#define COMMIT_HASH \"$(shell git rev-parse --short HEAD 2>/dev/null)"\" >> build/defs.h
	@$(ECHO) "#endif" >> build/defs.h

####################################################################################################
# Start all generators
####################################################################################################
.PHONY : rungens
rungens :
	@$(RUNGENS) $(GENERATOR)

####################################################################################################
# Copy git hooks to git repository
####################################################################################################
.PHONY : apply_git_hooks
apply_git_hooks :
	@$(GIT_HOOKS)

####################################################################################################
# makes dependences
####################################################################################################
.PHONY : dependencies
dependencies :
	@$(ECHO) "Creating dependencies for '$(TARGET)' target..."
	@$(MKDIR) $(TARGET_PATH)
	@$(RM) $(TARGET_PATH)/*.*
	@$(ECHO) "" > $(TARGET_PATH)/$(DEP_FILE_NAME)
	@$(MKDEP)    -f $(TARGET_PATH)/$(DEP_FILE_NAME) -p $(OBJ_PATH)/ -o .$(OBJ_EXT) $(SEARCHPATH_TARGET) -Y -- $(CFLAGS_$(TARGET)) -- $(CSRC) $(CXXSRC) >/dev/null 2>&1
	@$(MKDEP) -a -f $(TARGET_PATH)/$(DEP_FILE_NAME) -p $(OBJ_PATH)/ -o .$(OBJ_EXT) $(SEARCHPATH_PROGRAMS) -Y -- $(CFLAGS_$(TARGET)) -- $(CSRC) $(CXXSRC) >/dev/null 2>&1
	@$(MKDEP) -a -f $(TARGET_PATH)/$(DEP_FILE_NAME) -p $(OBJ_PATH)/ -o .$(OBJ_EXT) $(SEARCHPATH_LIB) -Y -- $(CFLAGS_$(TARGET)) -- $(CSRC) $(CXXSRC) >/dev/null 2>&1
	@$(MKDEP) -a -f $(TARGET_PATH)/$(DEP_FILE_NAME) -p $(OBJ_PATH)/ -o .$(OBJ_EXT) $(SEARCHPATH_CORE) -Y -- $(CFLAGS_$(TARGET)) -- $(CSRC) $(CXXSRC) >/dev/null 2>&1
	@$(MKDEP) -a -f $(TARGET_PATH)/$(DEP_FILE_NAME) -p $(OBJ_PATH)/ -o .$(OBJ_EXT) $(SEARCHPATH_NOARCH) -Y -- $(CFLAGS_$(TARGET)) -- $(CSRC) $(CXXSRC) >/dev/null 2>&1
	@$(MKDEP) -a -f $(TARGET_PATH)/$(DEP_FILE_NAME) -p $(OBJ_PATH)/ -o .$(OBJ_EXT) $(SEARCHPATH_ARCH) -Y -- $(CFLAGS_$(TARGET)) -- $(CSRC) $(CXXSRC) >/dev/null 2>&1
	@$(ECHO) "$(foreach var,$(CSRC),\n$(OBJ_PATH)/$(var:.$(C_EXT)=.$(OBJ_EXT)) : $(var))" >> $(TARGET_PATH)/$(DEP_FILE_NAME)
	@$(ECHO) "$(foreach var,$(CXXSRC),\n$(OBJ_PATH)/$(var:.$(CXX_EXT)=.$(OBJ_EXT)) : $(var))" >> $(TARGET_PATH)/$(DEP_FILE_NAME)
	@$(ECHO) "$(foreach var,$(ASRC),\n$(OBJ_PATH)/$(var:.$(AS_EXT)=.$(OBJ_EXT)) : $(var))" >> $(TARGET_PATH)/$(DEP_FILE_NAME)

####################################################################################################
# linking rules
####################################################################################################
.PHONY : linkobjects
linkobjects :
	@$(ECHO) "Linking..."
	@$(LD) $(foreach var,$(OBJECTS),$(OBJ_PATH)/$(var)) $(LFLAGS) -o $(TARGET_PATH)/$(PROJECT).elf

####################################################################################################
# build objects
####################################################################################################
.PHONY : buildobjects buildobjects_$(TARGET)
buildobjects :
	@$(ECHO) "Starting building objects up to $(THREAD) threads..."
	@$(MAKE) -s -j$(THREAD) -f$(THIS_MAKEFILE) buildobjects_$(TARGET)

buildobjects_$(TARGET) :$(foreach var,$(OBJECTS),$(OBJ_PATH)/$(var))

####################################################################################################
# rule used to compile object files from c sources
####################################################################################################
$(OBJ_PATH)/%.$(OBJ_EXT) : %.$(C_EXT) $(THIS_MAKEFILE)
	@$(ECHO) "Compiling: $<"
	@$(MKDIR) $(dir $@)
	@$(CC) $(CFLAGS) $(SEARCHPATH) $< -o $@
	$(FIND_GLOBAL_VARS_LIBS_PROGS)

####################################################################################################
# rule used to compile object files from C++ sources
####################################################################################################
$(OBJ_PATH)/%.$(OBJ_EXT) : %.$(CXX_EXT) $(THIS_MAKEFILE)
	@$(ECHO) "Compiling: $<"
	@$(MKDIR) $(dir $@)
	@$(CXX) $(CXXFLAGS) $(SEARCHPATH) $< -o $@
	$(FIND_GLOBAL_VARS_LIBS_PROGS)

####################################################################################################
# rule used to compile object files from assembler sources
####################################################################################################
$(OBJ_PATH)/%.$(OBJ_EXT) : %.$(AS_EXT) $(THIS_MAKEFILE)
	@$(ECHO) "Compiling: $<"
	@$(MKDIR) $(dir $@)
	@$(AS) $(AFLAGS) $(SEARCHPATH) $< -o $@

####################################################################################################
# clean target
####################################################################################################
.PHONY : cleantarget
cleantarget :
	@$(ECHO) "Cleaning target..."
	-@$(RM) -r $(OBJ_PATH) $(LST_PATH)
	-@$(RM) $(TARGET_DIR_NAME)/*.*

####################################################################################################
# clean all targets
####################################################################################################
.PHONY : clean
clean :
	@$(ECHO) "Deleting all build files..."
	-@$(RM) -r $(TARGET_DIR_NAME) ./doc/doxygen/html/ ./doc/doxygen/latex ./$(APP_LOC)/Makefile ./$(APP_LOC)/program_registration.c

####################################################################################################
# flash target CPU by using ./tools/flash.sh script
####################################################################################################
.PHONY : flash
flash:
	@$(FLASH_CPU)

.PHONY : install
install : flash

####################################################################################################
# reset target CPU by using ./tools/flash.sh script
####################################################################################################
.PHONY : reset
reset:
	@$(RESET_CPU)

####################################################################################################
# target used to create Release archive
####################################################################################################
.PHONY : release
release: clean
	@$(SHELL) $(RELEASEPKG)

####################################################################################################
# target used to Doxygen documentation
####################################################################################################
.PHONY : doc
doc:
	$(DOXYGEN)

####################################################################################################
# include all dependencies
####################################################################################################
-include $(TARGET_PATH)/$(DEP_FILE_NAME)
