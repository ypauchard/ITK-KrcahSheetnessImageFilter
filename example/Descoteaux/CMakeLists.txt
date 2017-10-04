# Executable
add_executable(DescoteauxSheetness main.cxx)
target_link_libraries(DescoteauxSheetness ${ITK_LIBRARIES} )
set_target_properties( DescoteauxSheetness
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)