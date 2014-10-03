
include_directories(include)

add_executable(KrcahSheetness main.cxx)

target_link_libraries(KrcahSheetness ${Glue} ${VTK_LIBRARIES} ${ITK_LIBRARIES} )