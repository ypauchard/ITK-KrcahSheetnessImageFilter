#include_directories(include)
add_executable(KrcahSheetness main.cxx)
target_link_libraries(KrcahSheetness ${ITK_LIBRARIES} )
set_target_properties( KrcahSheetness
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)