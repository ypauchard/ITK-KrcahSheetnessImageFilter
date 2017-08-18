# Executable
add_executable(DescoteauxSheetnessMax main.cxx)
target_link_libraries(DescoteauxSheetnessMax ${ITK_LIBRARIES} )
set_target_properties( DescoteauxSheetnessMax
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)