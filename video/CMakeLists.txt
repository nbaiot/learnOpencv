aux_source_directory(. SRCS)

add_library(common_video STATIC ${SRCS})

add_dependencies(common_video ffmpeg_wrapper)
