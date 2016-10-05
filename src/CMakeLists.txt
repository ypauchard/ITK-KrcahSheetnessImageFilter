
include_directories(include)

add_executable(KrcahSheetness main.cxx)

target_link_libraries(KrcahSheetness ${ITK_LIBRARIES} )

subdirs(GraphCut3D)