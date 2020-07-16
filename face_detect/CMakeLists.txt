aux_source_directory(. SRCS)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../ui UI)

add_executable(face_detect ${SRCS} ${UI})

add_dependencies(face_detect ${COMMON_DEPS})

target_link_libraries(face_detect

        ${QT_LIB}
        common_video ffmpeg_wrapper
        ${COMMON_LIBS}  ${FF_LIB} gflags )