cmake_minimum_required(VERSION 3.12)
project(learnOpencv)

set(CMAKE_CXX_STANDARD 17)

set(THIRD_PARTY_INSTALL_PATH /opt/third_party/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR})
set(THIRD_PARTY_INDEPENDENT_INSTALL FALSE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(cmake/generic.cmake)

include(cmake/external/gflags.cmake)
include(cmake/external/glog.cmake)
include(cmake/external/boost.cmake)

if (DEFINED ENV{OpenCV_HOME})
    set(OpenCV_DIR $ENV{OpenCV_HOME})
else()
    set(OpenCV_DIR /opt/third_party/Linux/x86_64/lib/cmake/opencv4)
endif ()

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


if (CUDA_FOUND)
    add_definitions(-DUSE_GPU)
endif ()

set(COMMON_DEPS gflags glog boost_system)

set(COMMON_LIBS ${COMMON_DEPS} pthread ${OpenCV_LIBS})

add_subdirectory(hello_opencv)