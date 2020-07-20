cmake_minimum_required(VERSION 3.12)
project(learn_opencv)

set(CMAKE_CXX_STANDARD 17)

set(THIRD_PARTY_INSTALL_PATH /opt/third_party/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR})
set(THIRD_PARTY_INDEPENDENT_INSTALL FALSE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(cmake/generic.cmake)

include(cmake/external/gflags.cmake)
include(cmake/external/glog.cmake)
include(cmake/external/boost.cmake)


# for ffmpeg
include(cmake/external/png.cmake)
include(cmake/external/freetype.cmake)
include(cmake/external/jpeg.cmake)
include(cmake/external/libyuv.cmake)
include(cmake/external/x264.cmake)
include(cmake/external/x265.cmake)
include(cmake/external/vpx.cmake)
include(cmake/external/lame.cmake)
include(cmake/external/fdk-aac.cmake)
include(cmake/external/opus.cmake)
include(cmake/external/ffmpeg.cmake)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (DEFINED ENV{OpenCV_HOME})
    set(OpenCV_DIR $ENV{OpenCV_HOME})
else ()
    set(OpenCV_DIR /opt/third_party/Linux/x86_64/lib/cmake/opencv4)
endif ()

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wl,-Bsymbolic")

set(FFMPEG_DEP_LIBS freetype png opus lame fdk-aac x264 x265 vpx
        -lasound -ldl -lnuma -lva -lvdpau -lva-drm -lva-x11 -lX11 -llzma -lz -lm -lbz2)

set(FF_LIB ${FFMPEG_LIBS} ${FFMPEG_DEP_LIBS})


find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

if (CUDA_FOUND)
    add_definitions(-DUSE_GPU)
    add_definitions(-DHAVE_CUDA=1)
endif ()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")

set(COMMON_DEPS glog boost_system boost_timer boost_chrono boost_thread boost_filesystem libyuv jpeg-turbo)

set(COMMON_LIBS ${OpenCV_LIBS} ${COMMON_DEPS} pthread)

set(QT_LIB  Qt5::Gui Qt5::Widgets Qt5::Qml Qt5::Quick Qt5::Multimedia Qt5::MultimediaWidgets)

include(cmake/qt5.cmake)
message(">>>>>>>>>>>>>>>>>>>>>>>>${OpenCV_LIBS}")
aux_source_directory(utils COMMON_UTILS)

add_subdirectory(ffmpeg)
add_subdirectory(video)
add_subdirectory(hello_opencv)
add_subdirectory(face_detect)
add_subdirectory(object_detect)