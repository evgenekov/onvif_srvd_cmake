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
#include "ServiceContext.h"
#include "armoury/ThreadWarden.hpp"
#include "armoury/files.hpp"
#include "armoury/logger.hpp"
#include "daemon.hpp"
#include "rtsp-streams.h"
#include "smacros.h"

// ---- gsoap ----
#include "DeviceBinding.nsmap"
#include "soapDeviceBindingService.h"
#include "soapMediaBindingService.h"
#include "soapPTZBindingService.h"

#define FOREACH_SERVICE(APPLY, soap)                                                                                     \
    APPLY(DeviceBindingService, soap)                                                                                    \
    APPLY(MediaBindingService, soap)                                                                                     \
    APPLY(PTZBindingService, soap)                                                                                       \
    /*                                                                                                                   \
     * If you need support for other services,                                                                           \
     * add the desired option to the macro FOREACH_SERVICE.                                                              \
     *                                                                                                                   \
     * Note: Do not forget to add the gsoap binding class for the service,                                               \
     * and the implementation methods for it, like for DeviceBindingService                                              \
                                                                                                                       \ \
                                                                                                                       \ \
                                                                                                                       \ \
            APPLY(ImagingBindingService, soap)                                                                           \
            APPLY(PTZBindingService, soap)                                                                               \
            APPLY(RecordingBindingService, soap)                                                                         \
            APPLY(ReplayBindingService, soap)                                                                            \
            APPLY(SearchBindingService, soap)                                                                            \
            APPLY(ReceiverBindingService, soap)                                                                          \
            APPLY(DisplayBindingService, soap)                                                                           \
            APPLY(EventBindingService, soap)                                                                             \
            APPLY(PullPointSubscriptionBindingService, soap)                                                             \
            APPLY(NotificationProducerBindingService, soap)                                                              \
            APPLY(SubscriptionManagerBindingService, soap)                                                               \
    */

#define DECLARE_SERVICE(service, soap) service service##_inst(soap);

#define DISPATCH_SERVICE(service, soap)                                                                                \
    else if (service##_inst.dispatch() != SOAP_NO_METHOD)                                                              \
    {                                                                                                                  \
        soap_send_fault(soap);                                                                                         \
        soap_stream_fault(soap, std::cerr);                                                                            \
    }

struct GSoapWrapper
{
    GSoapWrapper() : ptr(soap_new(), &soap_free)
    {
    }

    soap const *getSoapPtr() const
    {
        return ptr.get();
    }

    soap *getSoapPtr()
    {
        return ptr.get();
    }

    ~GSoapWrapper()
    {
        soap_destroy(ptr.get()); // delete managed C++ objects
        soap_end(ptr.get());     // delete managed memory
    }

  private:
    std::unique_ptr<soap, decltype(&soap_free)> ptr;
};

class GSoapInstance
{
  public:
    static constexpr char const *g_workerName{"gsoap thread"};
    static constexpr bool g_copyDataOnce{true};
    struct Input
    {
    } dataIn;
    struct Output
    {
    } dataOut;

    int work();

    GSoapInstance(ServiceContext service_ctx);
    ~GSoapInstance();

    void checkServiceCtx();

  private:
    // std::unique_ptr<struct soap> soapInstanceUnq;
    // struct soap *soapInstance;
    ServiceContext serviceCtx;
    GSoapWrapper gSoap;
    DeviceBindingService DeviceBindingService_inst;
    MediaBindingService MediaBindingService_inst;
    PTZBindingService PTZBindingService_inst;
};

GSoapInstance::GSoapInstance(ServiceContext service_ctx)
    : serviceCtx(std::move(service_ctx)), gSoap{}, DeviceBindingService_inst{gSoap.getSoapPtr()},
      MediaBindingService_inst{gSoap.getSoapPtr()}, PTZBindingService_inst{gSoap.getSoapPtr()}
{

    if (!gSoap.getSoapPtr())
        throw std::out_of_range("soap context is empty");

    gSoap.getSoapPtr()->bind_flags = SO_REUSEADDR;

    if (!soap_valid_socket(soap_bind(gSoap.getSoapPtr(), NULL, serviceCtx.port, 10)))
    {
        soap_stream_fault(gSoap.getSoapPtr(), std::cerr);
        exit(EXIT_FAILURE);
    }

    // gSoap.getSoapPtr()->accept_timeout = 1; // timeout in sec
    gSoap.getSoapPtr()->send_timeout = 3; // timeout in sec
    gSoap.getSoapPtr()->recv_timeout = 3; // timeout in sec

    // save pointer of service_ctx in soap
    gSoap.getSoapPtr()->user = (void *)&serviceCtx;

    // verify serviceCtx has been stored in the class.
    checkServiceCtx();

    FOREACH_SERVICE(DECLARE_SERVICE, gSoap.getSoapPtr())
}

GSoapInstance::~GSoapInstance()
{
    return;
}

int GSoapInstance::work()
{
    // wait new client
    if (!soap_valid_socket(soap_accept(gSoap.getSoapPtr())))
        sleep(20);
    {
        arms::log<arms::LOG_INFO>("SOAP Invalid Socket");
        soap_stream_fault(gSoap.getSoapPtr(), std::cerr);
    }

    // process service
    if (soap_begin_serve(gSoap.getSoapPtr()))
    {
        arms::log<arms::LOG_INFO>("Process Service");
        soap_stream_fault(gSoap.getSoapPtr(), std::cerr);
    }
    FOREACH_SERVICE(DISPATCH_SERVICE, gSoap.getSoapPtr())
    else
    {
        arms::log<arms::LOG_INFO>("Unknown service");
        return 1;
    }

    arms::log<arms::LOG_INFO>("Soap Destroy");
    soap_destroy(gSoap.getSoapPtr()); // delete managed C++ objects
    soap_end(gSoap.getSoapPtr());     // delete managed memory
    return 0;
}

void GSoapInstance::checkServiceCtx(void)
{
    if (serviceCtx.eth_ifs.empty())
        throw std::runtime_error("Error: not set no one ehternet interface more details see opt --ifs\n");

    if (serviceCtx.scopes.empty())
        throw std::runtime_error("Error: not set scopes more details see opt --scope\n");

    if (serviceCtx.get_profiles().empty())
        throw std::runtime_error("Error: not set no one profile more details see --help\n");
}

RTSPStream rtspStreams;
Daemon onvifDaemon;

void processing_cfg(Configuration const &configStruct, ServiceContext &service_ctx)
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

int main(int argc, char *argv[])
{

    arms::signals::registerThreadInterruptSignal();
    // Force STDIO to display debugging messages

    std::optional<std::string> const configFile{arms::files::findConfigFile("/etc/onvif_srvd/config.cfg")};
    Configuration const configStruct{configFile};

    DEBUG_MSG("processing_cfg\n");
    ServiceContext service_ctx{};
    processing_cfg(configStruct, service_ctx);

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
