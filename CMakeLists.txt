cmake_minimum_required(VERSION 3.7)

set(VITASDK_CMAKE_FILE "$ENV{VITASDK}/share/vita.cmake" CACHE PATH "VitaSDK CMake functions file")
include("${VITASDK_CMAKE_FILE}" REQUIRED)
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1 -d ATTRIBUTE2=12")

project(Sonic12-Vita)

file(GLOB RSDK_SRC
    RSDKv4/*.cpp
)

add_executable(RSDKv4
    dependencies/all/tinyxml2/tinyxml2.cpp
    ${RSDK_SRC}
    RSDKv4/NativeObjects/All.cpp
)

target_include_directories( RSDKv4 PRIVATE
    RSDKv4
    RSDKv4/NativeObjects
    dependencies/all/stb-image
    dependencies/all/tinyxml2
    "$ENV{VITASDK}/arm-vita-eabi/include/SDL2"
    "$ENV{VITASDK}/arm-vita-eabi/include/freetype2"
)

target_compile_options(RSDKv4 PRIVATE -ffast-math -O3 -fsigned-char -fpermissive -std=c++17)

target_compile_definitions(RSDKv4 PUBLIC -DBASE_PATH="ux0:data/Sonic1/")

target_link_libraries(RSDKv4
    SDL2
    SDL2_image
    vorbisfile
    vorbis
    ogg
    mpg123
    FLAC
    mikmod
    opus
    freetype
    jpeg
    png
    theora
    SceMotion_stub
    SceCommonDialog_stub
    SceHid_stub
    SceGxm_stub
    SceAudioIn_stub
    SceAudio_stub
    ScePower_stub
    SceDisplay_stub
    SceCtrl_stub
    SceTouch_stub
    SceIofilemgr_stub
    SceSsl_stub
    SceSysmodule_stub
    pthread
    ${SDL2_LIBRARIES}
    gcc
    g
    m
)

vita_create_self(eboot.bin
    RSDKv4
    UNSAFE
    NOASLR
)

# Build VPK

set(VPK "Sonic1.vpk")
set(TITLE_NAME "Sonic 1")
set(TITLE_ID "SONIC0001")
set(TITLE_VER "01.30")

vita_create_vpk("${VPK}" "${TITLE_ID}" eboot.bin
  NAME "${TITLE_NAME}"
  VERSION "${TITLE_VER}"
  FILE vita/sonic1_livearea sce_sys
)
