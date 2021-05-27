#include <errno.h>
#include <getopt.h>
#include <libconfig.h++>
#include <libconfig.h>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "ConfigLoader.hpp"
#include "Configuration.hpp"
#include "GSoapService.hpp"
#include "armoury/ThreadWarden.hpp"
#include "daemon.hpp"
#include "rtsp-streams.h"
#include "smacros.h"
#include "DeviceBinding.nsmap"


void processing_cfg(Configuration const &configStruct, ServiceContext &service_ctx, RTSPStream &rtspStreams, Daemon &onvifDaemon)
{
    // New function to handle config file
    StreamProfile profile;
    RTSPStreamConfig rtspConfig;
    DaemonInfo daemonInfo;

    // Get Daemon info
    daemonInfo.set_pidFile(configStruct.pid_file);
    daemonInfo.set_logLevel(configStruct.logLevel);
    daemonInfo.set_logFile(configStruct.logFile);
    daemonInfo.set_logFileSizeMb(configStruct.logFileSizeMb);
    daemonInfo.set_logFileCount(configStruct.logFileCount);
    daemonInfo.set_logAsync(configStruct.logAsync);

    if (!onvifDaemon.SaveConfig(daemonInfo))
        onvifDaemon.daemon_error_exit("Can't save daemon info: %s\n", service_ctx.get_cstr_err());

    DEBUG_MSG("Configured Daemon\n");

    // ONVIF Service Options
    service_ctx.port = configStruct.port;
    service_ctx.user = configStruct.user.c_str();
    service_ctx.password = configStruct.password.c_str();
    service_ctx.manufacturer = configStruct.manufacturer.c_str();
    service_ctx.model = configStruct.model.c_str();
    service_ctx.firmware_version = configStruct.firmware_version.c_str();
    service_ctx.serial_number = configStruct.serial_number.c_str();
    service_ctx.hardware_id = configStruct.hardware_id.c_str();

    for (auto it = begin(configStruct.scopes); it != end(configStruct.scopes); ++it)
    {
        service_ctx.scopes.push_back(it->scopeUri);
    }

    service_ctx.eth_ifs.push_back(Eth_Dev_Param());
    if (service_ctx.eth_ifs.back().open(configStruct.interfaces.c_str()) != 0)
        onvifDaemon.daemon_error_exit("Can't open ethernet interface: %s - %m\n", configStruct.interfaces.c_str());

    if (!service_ctx.set_tz_format(configStruct.tz_format.c_str()))
        onvifDaemon.daemon_error_exit("Can't set tz_format: %s\n", service_ctx.get_cstr_err());

    DEBUG_MSG("Configured Service\n");

    // Onvif Media Profiles
    for (auto it = begin(configStruct.profiles); it != end(configStruct.profiles); ++it)
    {
        profile.set_name(it->name.c_str());
        profile.set_width(it->width.c_str());
        profile.set_height(it->height.c_str());
        profile.set_url(it->url.c_str());
        profile.set_snapurl(it->snapUrl.c_str());
        profile.set_type(it->type.c_str());

        if (!service_ctx.add_profile(profile))
            onvifDaemon.daemon_error_exit("Can't add Profile: %s\n", service_ctx.get_cstr_err());

        DEBUG_MSG("configured Media Profile %s\n", profile.get_name().c_str());
        profile.clear();
    }

    // RTSP Streaming Configuration
    for (auto it = begin(configStruct.rtspStreams); it != end(configStruct.rtspStreams); ++it)
    {
        rtspConfig.set_pipeline(it->pipeline.c_str());
        rtspConfig.set_udpPort(it->udpPort.c_str());
        rtspConfig.set_tcpPort(it->tcpPort.c_str());
        rtspConfig.set_rtspUrl(it->rtspUrl.c_str());
        rtspConfig.set_testStream(it->testStream);
        rtspConfig.set_testStreamSrc(it->testStreamSrc.c_str());

        if (!rtspStreams.AddStream(rtspConfig))
            onvifDaemon.daemon_error_exit("Can't add Stream: %s\n", rtspStreams.get_cstr_err());

        DEBUG_MSG("configured Media Profile %s\n", rtspConfig.get_rtspUrl().c_str());
        rtspConfig.clear();
    }
}

int main()
{
    arms::signals::registerThreadInterruptSignal();

    std::optional<std::string> const configFile{arms::files::findConfigFile("/etc/onvif_srvd/config.cfg")};
    Configuration const configStruct{configFile};
    ServiceContext service_ctx{};
    RTSPStream rtspStreams{};
    Daemon onvifDaemon;

    DEBUG_MSG("processing_cfg\n");

    processing_cfg(configStruct, service_ctx, rtspStreams, onvifDaemon);

    api::StreamSettings settings;
    arms::log<arms::LOG_CRITICAL>(settings.toJsonString());

    // Set up two RTSP test card streams to run forever
    arms::logger::setupLogging(onvifDaemon.GetDaemonInfo().get_logLevel(), onvifDaemon.GetDaemonInfo().get_logAsync(),
                               onvifDaemon.GetDaemonInfo().get_logFile(),
                               onvifDaemon.GetDaemonInfo().get_logFileSizeMb(),
                               onvifDaemon.GetDaemonInfo().get_logFileCount());
    arms::log<arms::LOG_INFO>("Logging Enabled");

    auto addedStreams = rtspStreams.get_streams();
    arms::log<arms::LOG_INFO>("Found {} Streams", addedStreams.size());
    // std::vector<std::thread> threads;

    //    for( auto it = addedStreams.cbegin(); it != addedStreams.cend(); ++it )
    //    {
    //        std::stringstream ss;

    //        if(it->second.get_testStream())
    //        {
    //            ss << "\"( " << it->second.get_testStreamSrc() << it->second.get_pipeline();
    //        }
    //        else
    //        {
    //            ss << "\"( -v udpsrc port=" << it->second.get_udpPort() << " ! rtpjitterbuffer"  <<
    //            it->second.get_pipeline();
    //        }

    //        std::string s = ss.str();

    //        arms::log<arms::LOG_INFO>("Test Stream {}", s);
    //        threads.push_back(std::thread(&RTSPStream::InitRtspStream, s, it->second.get_tcpPort(),
    //        it->second.get_rtspUrl()));
    //    }


    arms::signals::registerThreadInterruptSignal();
    arms::ThreadWarden<GSoapInstance, ServiceContext> gSoapInstance{service_ctx};
    gSoapInstance.start();

    for (int i{}; i < 10; ++i)
    {
        gSoapInstance.checkAndRestartOnFailure();
        sleep(1);
    }
    arms::log<arms::LOG_INFO>("Attempting to stop thread");
    gSoapInstance.stop();
    arms::log<arms::LOG_INFO>("Stopped");

    return EXIT_FAILURE; // Error, normal exit from the main loop only through the signal handler.
}
