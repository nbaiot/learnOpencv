aux_source_directory(. SRCS)

add_library(ffmpeg_wrapper STATIC ${SRCS})

add_dependencies(ffmpeg_wrapper ${FFMPEG_LIBS} boost_system)
