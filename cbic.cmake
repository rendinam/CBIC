
set(EXENAME cbic)
set(SRC_FILES main/cbic.c)


set(LINK_LIBS
  CBIC
  CBPM-TSHARC
  BeamInstSupport
  mpmnet
  cbi_net
  cbpmfio
  c_utils
  ${GTK2_LIBRARIES}
)






