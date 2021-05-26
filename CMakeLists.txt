cmake_minimum_required(VERSION 3.7...3.20)
include(ExternalProject)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

set( CMAKE_VERBOSE_MAKEFILE on )
#set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
#set(CMAKE_C_COMPILER "/usr/bin/clang")


project(onvif_srvd)
                    
set(DAEMON_NAME "onvif_srvd")
set(DAEMON_MAJOR_VERSION 0)
set(DAEMON_MINOR_VERSION 0)
set(DAEMON_PATCH_VERSION 1)
set(DAEMON_PID_FILE_NAME "${DAEMON_NAME}.pid")
set(DAEMON_LOG_FILE_NAME "${DAEMON_NAME}.log")
set(DAEMON_NO_CHDIR 1)
set(DAEMON_NO_CLOSE_STDIO 0)

set(GSOAP_VERSION "2.8.113")
set(GSOAP_SRC_URL "https://sourceforge.net/projects/gsoap2/files/gsoap_${GSOAP_VERSION}.zip/download")

set(GSOAP_SRC_DIR "${PROJECT_SOURCE_DIR}/gsoap-2.8")
set(GSOAP_DIR "${GSOAP_SRC_DIR}/gsoap")
set(GSOAP_CUSTOM_DIR "${GSOAP_DIR}/custom")
set(GSOAP_PLUGIN_DIR "${GSOAP_DIR}/plugin")
set(GSOAP_IMPORT_DIR "${GSOAP_DIR}/import")
set(SOAPCPP2 "${GSOAP_DIR}/src/soapcpp2")
set(WSDL2H "${GSOAP_DIR}/wsdl/wsdl2h")
set(GENERATED_DIR "${PROJECT_SOURCE_DIR}/generated")
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(PATCH_DIR "${PROJECT_SOURCE_DIR}/patches")

set(ARMOURY_SRC "${PROJECT_SOURCE_DIR}/external/armoury/include")
set(ARMOURY_THIRD_PARTY_SRC "${PROJECT_SOURCE_DIR}/external/armoury/third_party/include")


message("SOFTWARE_VERSION is " ${DAEMON_MAJOR_VERSION} ${DAEMON_MINOR_VERSION} ${DAEMON_PATCH_VERSION})


add_definitions( -DDAEMON_NAME="${DAEMON_NAME}"
                 -DDAEMON_MAJOR_VERSION=${DAEMON_MAJOR_VERSION}
                 -DDAEMON_MINOR_VERSION=${DAEMON_MINOR_VERSION}
                 -DDAEMON_PATCH_VERSION=${DAEMON_PATCH_VERSION}
                 -DDAEMON_PID_FILE_NAME="${DAEMON_PID_FILE_NAME}"
                 -DDAEMON_LOG_FILE_NAME="${DAEMON_LOG_FILE_NAME}"
                 -DDAEMON_NO_CHDIR=${DAEMON_NO_CHDIR}
                 -DDAEMON_NO_CLOSE_STDIO=${DAEMON_NO_CLOSE_STDIO}
                 -DWITH_DOM
                 -DWITH_OPENSSL)
               
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(WARN_FLAGS "-Wall -Wno-long-long -Wextra -Wnon-virtual-dtor ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARN_FLAGS} -fcf-protection=none")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARN_FLAGS} -fcf-protection=none")
set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -O2 -mtune=generic")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -mtune=generic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")


# Remember to set env for both make an running:
# export TSAN_OPTIONS="second_deadlock_stack=1 tsan-handle-cxx-exceptions=1 ignore_noninstrumented_modules=1"
# TSAN appears not to work with exceptions, as per some wiki.
if (0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=thread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
endif ()
# Requires package LLVM for printing symbols, compaitble with gcc.
# export ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer
# export ASAN_OPTIONS=symbolize=1
# export ASAN_OPTIONS=fast_unwind_on_malloc=0
if (0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope")
endif()

if (0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
endif ()

set(GENERATED_FILES
         ${PROJECT_SOURCE_DIR}/generated/soapDeviceBindingService.cpp
         ${PROJECT_SOURCE_DIR}/generated/soapMediaBindingService.cpp
         ${PROJECT_SOURCE_DIR}/generated/soapPTZBindingService.cpp
         ${PROJECT_SOURCE_DIR}/generated/soapC.cpp
         ${GSOAP_PLUGIN_DIR}/wsseapi.c
         ${GSOAP_PLUGIN_DIR}/mecevp.c
         ${GSOAP_PLUGIN_DIR}/smdevp.c
         ${GSOAP_PLUGIN_DIR}/wsaapi.c
         ${GSOAP_CUSTOM_DIR}/struct_timeval.c
         ${GSOAP_DIR}/stdsoap2.cpp
         ${GSOAP_DIR}/dom.cpp
         ${GSOAP_CUSTOM_DIR}/duration.c)

ExternalProject_Add(
    gsoap_src
    URL ${GSOAP_SRC_URL}
    DOWNLOAD_NAME gsoap.zip
    BUILD_BYPRODUCTS "${GENERATED_FILES}"
    SOURCE_DIR "${GSOAP_SRC_DIR}"
    BUILD_COMMAND bash -c "${PROJECT_SOURCE_DIR}/build-gsoap.sh ${PROJECT_SOURCE_DIR}/gsoap-2.8 ${PATCH_DIR}"
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND cmake -E echo "Skipping install step."
    
    BUILD_IN_SOURCE 1
)




find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCONFIGXX REQUIRED libconfig++)
pkg_check_modules(LIBCONFIG REQUIRED libconfig)
pkg_check_modules(LIBMOSQUITTO REQUIRED libmosquitto)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=1.4)
pkg_check_modules(GSTRTSPSERVER REQUIRED gstreamer-rtsp-server-1.0)
pkg_check_modules(SSL REQUIRED openssl)
pkg_check_modules(ZLIB REQUIRED zlib)
set(CMAKE_THREAD_LIBS_INIT "-lpthread")

include_directories(
    ${GSOAP_DIR}
    ${ARMOURY_SRC}
    ${ARMOURY_THIRD_PARTY_SRC}
    ${CMAKE_SOURCE_DIR}/generated/
    ${CMAKE_SOURCE_DIR}/src/
    ${GSOAP_CUSTOM_DIR}
    ${GSOAP_PLUGIN_DIR}
    ${GSOAP_IMPORT_DIR}
)


file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src)
add_subdirectory(src/ ${CMAKE_BINARY_DIR}/bin/)
