
set(LIBNAME CBIC)
cmake_minimum_required(VERSION $ENV{ACC_CMAKE_VERSION})

find_package(GTK2)

set(INC_DIRS 
  ../include
  include
  comm_include
  ../c_utils/include
  ../mpmnet/include
  ../cbi_net/include
  ../cbpmfio/include
  ../BeamInstSupport/hardware_include
  ../BeamInstSupport/include
  ../BeamInstSupport/comm_include
  ../BeamInstSupport/cesr_intf_include
  ../CBPM-TSHARC/include
  ../CBPM-TSHARC/comm_include
  ${GTK2_INCLUDE_DIRS}
)

set(SRC_DIRS
  management
  plotting_gp
  system
  utils
)

set(EXE_SPECS cbic.cmake)

include($ENV{ACC_BUILD_SYSTEM}/Master.cmake)


