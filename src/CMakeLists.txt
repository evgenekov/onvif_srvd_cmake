message("Config Src")

#Set source files
file( GLOB SRCFILES
         ${PROJECT_SOURCE_DIR}/src/*.cpp
)

file( GLOB HDRFILES
        ${PROJECT_SOURCE_DIR}/src/*.hpp
)

set( CLANG_TOOLS_FILES ${CLANG_TOOLS_FILES} ${SRCFILES} ${HDRFILES} PARENT_SCOPE)
link_directories("${PROJECT_BINARY_DIR}/lib")
add_executable(onvif_srvd ${SRCFILES})

target_link_libraries(onvif_srvd
    ${GSTREAMER_LIBRARIES}
    ${GSTREAMER_APP_LIBRARIES}
    ${GSTREAMER_VIDEO_LIBRARIES}
    ${LIBCONFIGXX_LIBRARIES}
    ${LIBMOSQUITTO_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_include_directories(onvif_srvd BEFORE PRIVATE
    ${LIBCONFIGXX_INCLUDE_DIRS}
    ${LIBMOSQUITTO_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS onvif_srvd RUNTIME DESTINATION ${CMAKE_BUILD_DIR}/bin)

add_custom_target(run
    COMMAND onvif_srvd
    DEPENDS onvif_srvd
    WORKING_DIRECTORY ${CMAKE_BUILD_DIR}
)

message("Config Src Complete")
