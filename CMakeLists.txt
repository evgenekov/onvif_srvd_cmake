cmake_minimum_required(VERSION 3.7...3.20)
include(ExternalProject)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

set( CMAKE_VERBOSE_MAKEFILE on )
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

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
add_compile_options(-O2  -Wall  -pipe)

ExternalProject_Add(
    gsoap_src
    URL ${GSOAP_SRC_URL}
    DOWNLOAD_NAME gsoap.zip
    BUILD_BYPRODUCTS ""
    SOURCE_DIR "${GSOAP_SRC_DIR}"
    BUILD_COMMAND bash -c "${PROJECT_SOURCE_DIR}/build-gsoap.sh"
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
