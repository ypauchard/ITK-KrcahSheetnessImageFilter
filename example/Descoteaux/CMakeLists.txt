# ITK
message(WARNING "ITK must be compiled with Module_LesionSizingToolkit turned on to use Descoteaux Sheetness")

# Executable
add_executable(DescoteauxSheetness main.cxx)
target_link_libraries(DescoteauxSheetness ${ITK_LIBRARIES} )