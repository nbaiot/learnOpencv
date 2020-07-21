aux_source_directory(. SRCS)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../ui UI)

add_executable(open_pose ${SRCS} ${UI})

add_dependencies(open_pose ${COMMON_DEPS})

target_link_libraries(open_pose
        ${QT_LIB}
        common_video ffmpeg_wrapper
        ${COMMON_LIBS}  ${FF_LIB} gflags )