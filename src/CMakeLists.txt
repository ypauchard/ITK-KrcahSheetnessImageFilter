
include_directories(include)

add_executable(Testbench main.cxx)

target_link_libraries(Testbench ${Glue} ${VTK_LIBRARIES} ${ITK_LIBRARIES} )
