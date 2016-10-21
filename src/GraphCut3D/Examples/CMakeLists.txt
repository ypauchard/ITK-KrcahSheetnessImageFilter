ADD_EXECUTABLE(KrcahGraphcut KrcahGraphcut.cpp)
TARGET_LINK_LIBRARIES(KrcahGraphcut
${ITK_LIBRARIES}
${ImageGraphCut3DSegmentation_libraries})
