aux_source_directory(. SRCS)

add_executable(face_detect ${SRCS})

add_dependencies(face_detect ${COMMON_DEPS})

target_link_libraries(face_detect
        common_video ffmpeg_wrapper
        ${COMMON_LIBS} ${FF_LIB} gflags)