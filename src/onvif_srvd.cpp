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


int main()
{
    arms::signals::registerThreadInterruptSignal();

    std::optional<std::string> const configFile{arms::files::findConfigFile("/etc/onvif_srvd/config.cfg")};
    Configuration const configStruct{configFile};
    ServiceContext service_ctx{configStruct};
    Daemon onvifDaemon;

    std::list<GStreamerRTSP> listOfStreams;
    arms::signals::registerThreadInterruptSignal();

    // Set up two RTSP test card streams to run forever
    arms::logger::setupLogging(configStruct.logLevel, configStruct.logAsync,
                               configStruct.logFile, configStruct.logFileSizeMb,
                               configStruct.logFileCount);
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
