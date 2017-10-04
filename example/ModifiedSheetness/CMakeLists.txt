# Executable
add_executable(ModifiedSheetness main.cxx)
target_link_libraries(ModifiedSheetness ${ITK_LIBRARIES} )
set_target_properties( ModifiedSheetness
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)