aux_source_directory(. SRCS)

add_executable(hello_opencv ${SRCS})

add_dependencies(hello_opencv ${COMMON_DEPS})

target_link_libraries(hello_opencv ${OpenCV_LIBS} pthread)