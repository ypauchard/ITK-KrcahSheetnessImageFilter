# Descoteaux
if(NOT ECHO_ITK_WARNING)
  message(WARNING "ITK must be compiled with Module_LesionSizingToolkit turned on to use Descoteaux Sheetness")
  option(ECHO_ITK_WARNING "Echo warning about Module_LesionSizingToolkit" ON)
endif()

subdirs(Descoteaux DescoteauxWithScaling DescoteauxMax Krcah ModifiedSheetness)
