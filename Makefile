#=============================================================================
#
# File:     Makefile
#
# Purpose:  Makefile to build the CBIC library
#
# Author:   (From standard M.job template by M. Palmer)
#
# Acknowledgements:  Simon Patton's CLEO Makefile infrastructure
#                    Dave Sagan's descrip.mms 
#                    Mike Marsh and his makefiles
#=============================================================================


#-----------------------------------------------------------------------------
# Specify default source directories for code to be compiled and put into
# libraries, for module files, for code to be compiled into explicit object
# files, and initialization files to be copied to the config directory tree.
# Also specify lists of LOCAL directories to search for include files and 
# compiled module files (NOTE:  the directories $(CESR_INC) and $(CESR_MOD)
# are automatically included in this list by M.tail).  
#
# Multiple directories should be separated by spaces.
#
#
# WARNING:  DO NOT put a trailing SLASH on directory names
#
# WARNING:  EMPTY lists should have NO EXTRA SPACES after the :=
#           It is better to simply delete them.  The ones shown here 
#           are intended to show the full range of options available.
#
#
# CODE_SRC_DIRS   - Put a list of code sub-directories here
# MOD_SRC_DIRS    - Put a list of F90 module source sub-directories here
# OBJ_SRC_DIRS    - Put a list of sub-directories with code that should be 
#                   compiled into explicit .o files but NOT stored in the 
#                   archive library here (for instance, object files for 
#                   for programs).
# CONFIG_DIRS     - Put a list of sub-directories with initialization files
#                   here.   NOTE:  It is assumed that ALL config files in the
#                   CONFIG_DIRS are of the form *.*  This is to avoid copying
#                   of contained sub-directories (eg, the CVS sub-directory).
# LOCAL_INCS      - Local directories to search for include files
# LOCAL_MODS      - Local directories to search for F90 compiled module files
#-----------------------------------------------------------------------------
LIB_SRC_DIRS  := ./console_io ./management ./plotting_gp ./system ./utils
MOD_SRC_DIRS  :=
OBJ_SRC_DIRS  := ./main
CONFIG_DIRS   :=
LOCAL_INCS    := ./include CBPM-TSHARC/include CBPM-TSHARC/comm_include BeamInstSupport/cesr_intf_include BeamInstSupport/include BeamInstSupport/comm_include BeamInstSupport/hardware_include c_utils/include mpmnet/include cfortran/include cbi_net/include ../cbpmfio/include
LOCAL_MODS    :=

OBJ_OUT_DIR   := ../lib

M_FILE_LIST := M.cbic
#-----------------------------------------------------------------------------
# "EXTRA" variables can be specified at the command line or hardwired here.  
# These variables are automatically appended to the relevant search paths and
# lists in M.tail
#
# EXTRA_SRC      - extra code source directories
# EXTRA_MOD_SRC  - extra module source directories
# EXTRA_OBJ_SRC  - extra object source directories
# EXTRA_CONFIG   - extra configuration file directories
# EXTRA_INCS     - extra include file search directories
# EXTRA_MODS     - extra compile module search directories
# EXTRA_FFLAGS   - extra Fortran flags for compilation
# EXTRA_CFLAGS   - extra C flags for compilation 
# EXTRA_LFLAGS   - extra linker flags
#-----------------------------------------------------------------------------
EXTRA_SRC     :=
EXTRA_MOD_SRC :=
EXTRA_OBJ_SRC :=
EXTRA_CONFIG  :=
EXTRA_INCS    :=
EXTRA_MODS    :=
EXTRA_FFLAGS  :=
EXTRA_CFLAGS  := -trigraphs `pkg-config --cflags gtk+-2.0` -D_REENTRANT
EXTRA_LFLAGS  := -Wl,-Map,/dev/null `pkg-config --libs gtk+-2.0`


#------------------------------------------------
# Name of local libraries (standard and debug) 
#------------------------------------------------
ifeq "$(LIBNAME)" ""
  WHERE   := $(shell pwd)
  LIBNAME := $(notdir $(WHERE))
endif


#------------------------------------------------
# Include the standard CESR M.tail makefile
#------------------------------------------------
ifeq "$(DIST_BUILD)" "TRUE"
  include $(DIST_GMAKE)/M.tail
else
  include $(ACC_GMAKE)/M.tail
endif


