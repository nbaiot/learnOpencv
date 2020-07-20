aux_source_directory(. SRCS)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../ui UI)

add_executable(object_detect ${SRCS} ${UI})

add_dependencies(object_detect ${COMMON_DEPS})

target_link_libraries(object_detect
        ${QT_LIB}
        common_video ffmpeg_wrapper
        ${COMMON_LIBS}  ${FF_LIB} gflags )