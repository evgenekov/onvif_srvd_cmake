cmake_minimum_required(VERSION 3.7...3.20)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

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

set(GSOAP_INSTALL_DIR "./gsoap-2.8")
set(GSOAP_DIR "${GSOAP_INSTALL_DIR}/gsoap")
set(GSOAP_CUSTOM_DIR "${GSOAP_DIR}/custom")
set(GSOAP_PLUGIN_DIR "${GSOAP_DIR}/plugin")
set(GSOAP_IMPORT_DIR "${GSOAP_DIR}/import")


message("SOFTWARE_VERSION is " ${DAEMON_MAJOR_VERSION} ${DAEMON_MINOR_VERSION} ${DAEMON_PATCH_VERSION})


add_definitions( -DDAEMON_NAME=${DAEMON_NAME}
                 -DDAEMON_MAJOR_VERSION=${DAEMON_MAJOR_VERSION}
                 -DDAEMON_MINOR_VERSION=${DAEMON_MINOR_VERSION}
                 -DDAEMON_PATCH_VERSION=${DAEMON_PATCH_VERSION}
                 -DDAEMON_PID_FILE_NAME=${DAEMON_PID_FILE_NAME}
                 -DDAEMON_LOG_FILE_NAME=${DAEMON_LOG_FILE_NAME}
                 -DDAEMON_NO_CHDIR=${DAEMON_NO_CHDIR}
                 -DDAEMON_NO_CLOSE_STDIO=${DAEMON_NO_CLOSE_STDIO})
          
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-std=c++17 -O2  -Wall  -pipe")

function(download_file url filename)
message("Download File ${filename}")
if(NOT EXISTS ${filename})
  file(DOWNLOAD ${url} ${filename}
       SHOW_PROGRESS)
endif()
       
if(NOT EXISTS "${GSOAP_INSTALL_DIR}/README.txt")
  file(ARCHIVE_EXTRACT INPUT ${filename})
  
endif()

execute_process(COMMAND bash -c "${PROJECT_SOURCE_DIR}/build-gsoap.sh")
include_directories(${GSOAP_CUSTOM_DIR})
include_directories(${GSOAP_PLUGIN_DIR})
include_directories(${GSOAP_IMPORT_DIR})

endfunction(download_file)


download_file( "https://sourceforge.net/projects/gsoap2/files/gsoap_${GSOAP_VERSION}.zip/download" "gsoap.zip")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCONFIGXX REQUIRED libconfig++)
pkg_check_modules(LIBMOSQUITTO REQUIRED libmosquitto)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=1.4)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
include_directories(${CMAKE_SOURCE_DIR}/src/)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src)
add_subdirectory(src/ ${CMAKE_BINARY_DIR}/bin/)
