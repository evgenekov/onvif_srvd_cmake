message("Config onvif-srvd")

#Set source files
set( SRCFILES
         ${SRC_DIR}/daemon.cpp
         ${SRC_DIR}/onvif_srvd.cpp
         ${SRC_DIR}/eth_dev_param.cpp
         ${SRC_DIR}/ServiceContext.cpp
         ${SRC_DIR}/ServiceDevice.cpp
         ${SRC_DIR}/ServiceMedia.cpp
         ${SRC_DIR}/ServicePTZ.cpp
         ${SRC_DIR}/mosquitto_handler.c
         ${SRC_DIR}/rtsp-streams.cpp
         ${SRC_DIR}/Configuration.cpp
         ${SRC_DIR}/ConfigLoader.cpp
         ${SRC_DIR}/GSoapService.cpp
)

set( HDRFILES
         ${SRC_DIR}/daemon.hpp
         ${SRC_DIR}/eth_dev_param.h
         ${SRC_DIR}/ServiceContext.h
         ${SRC_DIR}/ServiceMedia.cpp
         ${SRC_DIR}/smacros.h
         ${SRC_DIR}/stools.h
         ${SRC_DIR}/mosquitto_handler.h
         ${SRC_DIR}/rtsp-streams.hpp
         ${SRC_DIR}/ConfigLoader.hpp
         ${SRC_DIR}/Configuration.hpp
         ${SRC_DIR}/GSoapService.hpp
         ${PROJECT_SOURCE_DIR}/generated/onvif.h
         ${PROJECT_SOURCE_DIR}/generated/soapDeviceBindingService.h
         ${PROJECT_SOURCE_DIR}/generated/soapMediaBindingService.h
         ${PROJECT_SOURCE_DIR}/generated/soapPTZBindingService.h
         ${PROJECT_SOURCE_DIR}/generated/soapStub.h
         ${PROJECT_SOURCE_DIR}/generated/soapH.h
)

set_source_files_properties(${SRCFILES} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${GENERATED_FILES} PROPERTIES LANGUAGE CXX)

set( CLANG_TOOLS_FILES ${CLANG_TOOLS_FILES} ${SRCFILES} ${HDRFILES} PARENT_SCOPE)

set(ALL_SOURCE ${SRCFILES} ${GENERATED_FILES})
message(${ALL_SOURCE})
add_custom_target(gen_gsoap_files DEPENDS ${GENERATED_FILES})
add_executable(onvif_srvd ${SRCFILES})
add_dependencies(onvif_srvd gsoap_src)
target_sources(onvif_srvd PUBLIC ${GENERATED_FILES})


target_link_libraries(onvif_srvd
    ${SSL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${LIBCONFIG_LIBRARIES}
    ${LIBCONFIGXX_LIBRARIES}
    ${GSTRTSPSERVER_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${LIBMOSQUITTO_LIBRARIES}
)

target_include_directories(onvif_srvd PUBLIC
    ${LIBCONFIGXX_INCLUDE_DIRS}
    ${LIBMOSQUITTO_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
 
)

message("Config onvif-srvd Complete")
