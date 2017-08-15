# ITK
message(WARNING "ITK must be compiled with Module_LesionSizingToolkit turned on to use Descoteaux Sheetness")

# Executable
add_executable(DescoteauxSheetnessMax main.cxx)
target_link_libraries(DescoteauxSheetnessMax ${ITK_LIBRARIES} )