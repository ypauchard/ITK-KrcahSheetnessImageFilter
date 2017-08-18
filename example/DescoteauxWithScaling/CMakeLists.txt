# Executable
add_executable(DescoteauxSheetnessScaling main.cxx)
target_link_libraries(DescoteauxSheetnessScaling ${ITK_LIBRARIES} )
set_target_properties( DescoteauxSheetnessScaling
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)