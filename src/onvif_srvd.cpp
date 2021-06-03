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
#include <list>

#include "ConfigLoader.hpp"
#include "Configuration.hpp"
#include "GSoapService.hpp"
#include "armoury/ThreadWarden.hpp"
#include "daemon.hpp"
#include "rtsp-streams.hpp"
#include "smacros.h"
#include "DeviceBinding.nsmap"


void processing_cfg(Configuration const &configStruct, ServiceContext &service_ctx, Daemon &onvifDaemon)
{
    // New function to handle config file
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
}

int main()
{
    arms::signals::registerThreadInterruptSignal();

    std::optional<std::string> const configFile{arms::files::findConfigFile("/etc/onvif_srvd/config.cfg")};
    Configuration const configStruct{configFile};
    ServiceContext service_ctx{configStruct};
    Daemon onvifDaemon;

    std::list<GStreamerRTSP> listOfStreams;
    arms::signals::registerThreadInterruptSignal();

    DEBUG_MSG("processing_cfg\n");
    processing_cfg(configStruct, service_ctx, onvifDaemon);

    arms::log("Configuring Profiles");

    // Set up two RTSP test card streams to run forever
    arms::logger::setupLogging(onvifDaemon.GetDaemonInfo().get_logLevel(), onvifDaemon.GetDaemonInfo().get_logAsync(),
                               onvifDaemon.GetDaemonInfo().get_logFile(),
                               onvifDaemon.GetDaemonInfo().get_logFileSizeMb(),
                               onvifDaemon.GetDaemonInfo().get_logFileCount());
    arms::log<arms::LOG_INFO>("Logging Enabled");

    arms::log<arms::LOG_INFO>("Found {} Streams in configStruct", configStruct.rtspStreams.size());
    // Create a thread for each stream and store it in a list
    for( auto itr : configStruct.rtspStreams )
    {
        listOfStreams.emplace_back(itr);
    }

    arms::ThreadWarden<GSoapInstance, ServiceContext> gSoapInstance{service_ctx};
    gSoapInstance.start();

    for (int i{}; i < 50; ++i)
    {
        gSoapInstance.checkAndRestartOnFailure();
        sleep(1);
    }

    gSoapInstance.stop();
    arms::log<arms::LOG_INFO>("Stopped");

    return EXIT_FAILURE; // Error, normal exit from the main loop only through the signal handler.
}
