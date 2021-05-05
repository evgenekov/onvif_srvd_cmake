message("Config Src")

#Set source files
set( SRCFILES
         ${SRC_DIR}/daemon.c
         ${SRC_DIR}/onvif_srvd.cpp
         ${SRC_DIR}/eth_dev_param.cpp
         ${SRC_DIR}/ServiceContext.cpp
         ${SRC_DIR}/ServiceDevice.cpp
         ${SRC_DIR}/ServiceMedia.cpp
         ${SRC_DIR}/ServicePTZ.cpp
         ${SRC_DIR}/mosquitto_handler.c
         ${SRC_DIR}/rtsp-streams.cpp
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
         ${GSOAP_CUSTOM_DIR}/duration.c
         
)


set( HDRFILES
         ${SRC_DIR}/daemon.h
         ${SRC_DIR}/eth_dev_param.h
         ${SRC_DIR}/ServiceContext.h
         ${SRC_DIR}/ServiceMedia.cpp
         ${SRC_DIR}/smacros.h
         ${SRC_DIR}/stools.h
         ${SRC_DIR}/mosquitto_handler.h
         ${SRC_DIR}/rtsp-streams.hpp
         ${PROJECT_SOURCE_DIR}/generated/onvif.h
         ${PROJECT_SOURCE_DIR}/generated/soapDeviceBindingService.h
         ${PROJECT_SOURCE_DIR}/generated/soapMediaBindingService.h
         ${PROJECT_SOURCE_DIR}/generated/soapPTZBindingService.h
         ${PROJECT_SOURCE_DIR}/generated/soapStub.h
         ${PROJECT_SOURCE_DIR}/generated/soapH.h
)

set_source_files_properties(${SRCFILES} PROPERTIES LANGUAGE CXX)

set( CLANG_TOOLS_FILES ${CLANG_TOOLS_FILES} ${SRCFILES} ${HDRFILES} PARENT_SCOPE)

add_executable(onvif_srvd ${SRCFILES})

add_dependencies(onvif_srvd gsoap_src)


target_link_libraries(onvif_srvd
    
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_APP_LIBRARIES}
    ${GSTREAMER_VIDEO_LIBRARIES}
    ${LIBCONFIGXX_LIBRARIES}
    ${LIBMOSQUITTO_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_include_directories(onvif_srvd PUBLIC
    ${LIBCONFIGXX_INCLUDE_DIRS}
    ${LIBMOSQUITTO_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
 
)





message("Config Src Complete")
