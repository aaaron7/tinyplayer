project(tinyplayer)
cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 11)
set(DEVELOPMENT_TEAM_ID GH246XP5QK)
FILE(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/core/*.*)
FILE(GLOB PLATFORM_FILES ${CMAKE_SOURCE_DIR}/platform/ios/**/*.*)
set(MOBILE_PLATFORM ON)

set(DEVICE_FAMILY "1")
macro(ADD_FRAMEWORK PROJECT_NAME FRAMEWORK_NAME)
    find_library(
        "FRAMEWORK_${FRAMEWORK_NAME}"
        NAMES ${FRAMEWORK_NAME}
        PATHS ${CMAKE_OSX_SYSROOT}/System/Library
        PATH_SUFFIXES Frameworks
        NO_DEFAULT_PATH
    )
    if(${FRAMEWORK_${FRAMEWORK_NAME}} STREQUAL FRAMEWORK_${FRAMEWORK_NAME}-NOTFOUND)
        MESSAGE(ERROR ": Framework ${FRAMEWORK_NAME} not found")
    else()
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${FRAMEWORK_${FRAMEWORK_NAME}})
        message(STATUS "find framework: ${FRAMEWORK_${FRAMEWORK_NAME}}")
        #include_directories(${PROJECT_NAME} ${FRAMEWORK_${FRAMEWORK_NAME}_INCLUDE_DIRS})
    endif()
endmacro(ADD_FRAMEWORK)




#ADD_DEFINE(IOS)
set(DEPLOYMENT_TARGET 8.0) 
include_directories(
    ${CMAKE_SOURCE_DIR}/../libs/include/
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/platform/ios/egl/
    ${CMAKE_SOURCE_DIR}/platform/ios/audio/

)

link_directories(
    ${CMAKE_SOURCE_DIR}/../libs/lib/
)

add_library(tinyplayer SHARED ${SRC_FILES} ${PLATFORM_FILES})
SET_XCODE_PROPERTY(tinyplayer CODE_SIGN_IDENTITY "iPhone Developer")
SET_XCODE_PROPERTY(tinyplayer DEVELOPMENT_TEAM ${DEVELOPMENT_TEAM_ID})
target_compile_options(tinyplayer PUBLIC "-fno-objc-arc")


ADD_FRAMEWORK(tinyplayer VideoToolBox)
ADD_FRAMEWORK(tinyplayer CoreMedia)
ADD_FRAMEWORK(tinyplayer CoreVideo)
ADD_FRAMEWORK(tinyplayer CoreFoundation)
ADD_FRAMEWORK(tinyplayer Security)
ADD_FRAMEWORK(tinyplayer AVFoundation)
ADD_FRAMEWORK(tinyplayer AudioToolBox)
ADD_FRAMEWORK(tinyplayer OpenGLES)
ADD_FRAMEWORK(tinyplayer UIKit)
ADD_FRAMEWORK(tinyplayer QuartzCore)
ADD_FRAMEWORK(tinyplayer Foundation)
ADD_FRAMEWORK(tinyplayer Accelerate)
link_libraries(avformat avcodec)
target_link_libraries(tinyplayer
"-Wl" 
avformat avcodec avdevice avfilter avutil swresample swscale
z bz2 iconv x264 fdk-aac
)
