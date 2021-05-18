#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <getopt.h>
#include <optional>
#include <stdexcept>
#include <libconfig.h>
#include <libconfig.h++>
#include <sstream>

#include "daemon.hpp"
#include "smacros.h"
#include "ServiceContext.h"
#include "rtsp-streams.h"
#include "ConfigLoader.hpp"
#include "Configuration.hpp"
#include "armoury/files.hpp"
#include "armoury/logger.hpp"
#include "armoury/ThreadWarden.hpp"

// ---- gsoap ----
#include "DeviceBinding.nsmap"
#include "soapDeviceBindingService.h"
#include "soapMediaBindingService.h"
#include "soapPTZBindingService.h"


#define FOREACH_SERVICE(APPLY, soap)                    \
        APPLY(DeviceBindingService, soap)               \
        APPLY(MediaBindingService, soap)                \
        APPLY(PTZBindingService, soap)                  \
/*
 * If you need support for other services,
 * add the desired option to the macro FOREACH_SERVICE.
 *
 * Note: Do not forget to add the gsoap binding class for the service,
 * and the implementation methods for it, like for DeviceBindingService



        APPLY(ImagingBindingService, soap)               \
        APPLY(PTZBindingService, soap)                   \
        APPLY(RecordingBindingService, soap)             \
        APPLY(ReplayBindingService, soap)                \
        APPLY(SearchBindingService, soap)                \
        APPLY(ReceiverBindingService, soap)              \
        APPLY(DisplayBindingService, soap)               \
        APPLY(EventBindingService, soap)                 \
        APPLY(PullPointSubscriptionBindingService, soap) \
        APPLY(NotificationProducerBindingService, soap)  \
        APPLY(SubscriptionManagerBindingService, soap)   \
*/


#define DECLARE_SERVICE(service, soap) service service ## _inst(soap);

#define DISPATCH_SERVICE(service, soap)                                  \
                else if (service ## _inst.dispatch() != SOAP_NO_METHOD) {\
                    soap_send_fault(soap);                               \
                    soap_stream_fault(soap, std::cerr);                  \
                }


class GSoapInstance
{
public:
    GSoapInstance(ServiceContext service_ctx);
    ~GSoapInstance();

    //struct soap getSoapContext() { return *soap; }
    void runSoapInstance();

private:
    struct soap *soapInstance;
    ServiceContext serviceCtx;
};

GSoapInstance::GSoapInstance(ServiceContext service_ctx) : serviceCtx(service_ctx)
{
    soapInstance = soap_new();

    if(!soapInstance)
        throw std::out_of_range("soap context is empty");

    soapInstance->bind_flags = SO_REUSEADDR;

    if( !soap_valid_socket(soap_bind(soapInstance, NULL, serviceCtx.port, 10)) )
    {
        soap_stream_fault(soapInstance, std::cerr);
        exit(EXIT_FAILURE);
    }

    soapInstance->send_timeout = 3; // timeout in sec
    soapInstance->recv_timeout = 3; // timeout in sec


    //save pointer of service_ctx in soap
    soapInstance->user = (void*)&serviceCtx;
}

GSoapInstance::~GSoapInstance()
{
    soap_destroy(soapInstance); // delete managed C++ objects
    soap_end(soapInstance);     // delete managed memory
}

void GSoapInstance::runSoapInstance()
{
    FOREACH_SERVICE(DECLARE_SERVICE, soapInstance)

    while( true )
    {
        // wait new client
        if( !soap_valid_socket(soap_accept(soapInstance)) )
        {
            arms::log<arms::LOG_DEBUG>("SOAP Valid Socket");
            soap_stream_fault(soapInstance, std::cerr);
            throw std::invalid_argument("gSoap invalid socket");
        }

        // process service
        if( soap_begin_serve(soapInstance) )
        {
            soap_stream_fault(soapInstance, std::cerr);
        }
        FOREACH_SERVICE(DISPATCH_SERVICE, soapInstance)
        else
        {
            arms::log<arms::LOG_DEBUG>("Unknown service");
            throw std::runtime_error("Unknown service");
        }

        soap_destroy(soapInstance); // delete managed C++ objects
        soap_end(soapInstance);     // delete managed memory
    }
}


//static struct soap *soap;

ServiceContext service_ctx;
RTSPStream rtspStreams;
Daemon onvifDaemon;



//void daemon_exit_handler(int sig)
//{
//    //Here we release resources

//    UNUSED(sig);
//    soap_destroy(soap); // delete managed C++ objects
//    soap_end(soap);     // delete managed memory
//    soap_free(soap);    // free the context


//    unlink(onvifDaemon.GetDaemonInfo().get_pidFile().c_str());


//    exit(EXIT_SUCCESS); // good job (we interrupted (finished) main loop)
//}


//void init_signals(void)
//{
//    onvifDaemon.set_sig_handler(SIGINT,  daemon_exit_handler); //for Ctlr-C in terminal for debug (in debug mode)
//    onvifDaemon.set_sig_handler(SIGTERM, daemon_exit_handler);

//    signal(SIGCHLD, SIG_IGN); // ignore child
//    signal(SIGTSTP, SIG_IGN); // ignore tty signals
//    signal(SIGTTOU, SIG_IGN);
//    signal(SIGTTIN, SIG_IGN);
//    signal(SIGHUP,  SIG_IGN);
//}


const char* get_cfg_string(const char* setting, config_t config)
{
    const char *str;
    if(!config_lookup_string(&config, setting, &str))
    {
        DEBUG_MSG("No value found in config for %s.\n", setting);
        str = NULL;
    }
    return str;
}


int get_cfg_int(const char* setting, config_t config)
{
    int value;
    if(!config_lookup_int(&config, setting, &value))
    {
        DEBUG_MSG("No value found in config for %s.\n", setting);
        value = 0;
    }
    return value;
}


void processing_cfg(Configuration configStruct)
{
    // New function to handle config file
    config_t config;
    config_init(&config);
    StreamProfile  profile;
    RTSPStreamConfig rtspConfig;
    DaemonInfo daemonInfo;
    
    // Get Daemon info
    daemonInfo.set_pidFile(configStruct.pid_file);
    daemonInfo.set_logLevel(configStruct.logLevel);
    daemonInfo.set_logFile(configStruct.logFile);
    daemonInfo.set_logFileSizeMb(configStruct.logFileSizeMb);
    daemonInfo.set_logFileCount(configStruct.logFileCount);
    daemonInfo.set_logAsync(configStruct.logAsync);
    
    if( !onvifDaemon.SaveConfig(daemonInfo) )
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

    for (auto it = begin(configStruct.scopes); it != end(configStruct.scopes); ++it )
    {
        service_ctx.scopes.push_back(it->scopeUri);
    }

    service_ctx.eth_ifs.push_back(Eth_Dev_Param());
    if( service_ctx.eth_ifs.back().open(configStruct.interfaces.c_str()) != 0 )
        onvifDaemon.daemon_error_exit("Can't open ethernet interface: %s - %m\n", configStruct.interfaces.c_str());

    if( !service_ctx.set_tz_format(configStruct.tz_format.c_str()) )
        onvifDaemon.daemon_error_exit("Can't set tz_format: %s\n", service_ctx.get_cstr_err());

    DEBUG_MSG("Configured Service\n");

    // Onvif Media Profiles
    for (auto it = begin(configStruct.profiles); it != end(configStruct.profiles); ++it )
    {
        profile.set_name(it->name.c_str());
        profile.set_width(it->width.c_str());
        profile.set_height(it->height.c_str());
        profile.set_url(it->url.c_str());
        profile.set_snapurl(it->snapUrl.c_str());
        profile.set_type(it->type.c_str());

        if( !service_ctx.add_profile(profile) )
            onvifDaemon.daemon_error_exit("Can't add Profile: %s\n", service_ctx.get_cstr_err());

        DEBUG_MSG("configured Media Profile %s\n", profile.get_name().c_str());
        profile.clear();

    }

    // RTSP Streaming Configuration
    for (auto it = begin(configStruct.rtspStreams); it != end(configStruct.rtspStreams); ++it )
    {
        rtspConfig.set_pipeline(it->pipeline.c_str());
        rtspConfig.set_udpPort(it->udpPort.c_str());
        rtspConfig.set_tcpPort(it->tcpPort.c_str());
        rtspConfig.set_rtspUrl(it->rtspUrl.c_str());
        rtspConfig.set_testStream(it->testStream);
        rtspConfig.set_testStreamSrc(it->testStreamSrc.c_str());

        if( !rtspStreams.AddStream(rtspConfig) )
            onvifDaemon.daemon_error_exit("Can't add Stream: %s\n", rtspStreams.get_cstr_err());

        DEBUG_MSG("configured Media Profile %s\n", rtspConfig.get_rtspUrl().c_str());
        rtspConfig.clear();
    }
}


//void check_service_ctx(void)
//{
//    if(service_ctx.eth_ifs.empty())
//        onvifDaemon.daemon_error_exit("Error: not set no one ehternet interface more details see opt --ifs\n");


//    if(service_ctx.scopes.empty())
//        onvifDaemon.daemon_error_exit("Error: not set scopes more details see opt --scope\n");


//    if(service_ctx.get_profiles().empty())
//        onvifDaemon.daemon_error_exit("Error: not set no one profile more details see --help\n");
//}


//void init_gsoap(void)
//{
//    soap = soap_new();

//    if(!soap)
//        onvifDaemon.daemon_error_exit("Can't get mem for SOAP\n");


//    soap->bind_flags = SO_REUSEADDR;

//    if( !soap_valid_socket(soap_bind(soap, NULL, service_ctx.port, 10)) )
//    {
//        soap_stream_fault(soap, std::cerr);
//        exit(EXIT_FAILURE);
//    }

//    soap->send_timeout = 3; // timeout in sec
//    soap->recv_timeout = 3; // timeout in sec


//    //save pointer of service_ctx in soap
//    soap->user = (void*)&service_ctx;
//}


//void init(void *data)
//{
//    UNUSED(data);
//    init_signals();
//    check_service_ctx();
//    init_gsoap();
//}


int main(int argc, char *argv[])
{

    arms::signals::registerThreadInterruptSignal();
    // Force STDIO to display debugging messages
    
    std::optional<std::string> const configFile{arms::files::findConfigFile("/etc/onvif_srvd/config.cfg")};
    Configuration const configStruct{configFile};
    
    DEBUG_MSG("processing_cfg\n");
    processing_cfg(configStruct);
    
/*    arms::ThreadWarden<RTSPThread, RTSPStreamConfig> thread{rtspStreams.getStream("/right").value()};
    thread.start();
    sleep(3);
    thread.stop()*/;
    
    api::StreamSettings settings;
    arms::log<arms::LOG_CRITICAL>(settings.toJsonString());
    
//    onvifDaemon.daemonize2(init, NULL);

    // Set up two RTSP test card streams to run forever
    arms::logger::setupLogging(onvifDaemon.GetDaemonInfo().get_logLevel(), onvifDaemon.GetDaemonInfo().get_logAsync(), onvifDaemon.GetDaemonInfo().get_logFile(), onvifDaemon.GetDaemonInfo().get_logFileSizeMb(), onvifDaemon.GetDaemonInfo().get_logFileCount());
    arms::log<arms::LOG_INFO>("Logging Enabled");

    auto addedStreams = rtspStreams.get_streams();
    arms::log<arms::LOG_INFO>("Found {} Streams", addedStreams.size());
    std::vector<std::thread> threads;

    for( auto it = addedStreams.cbegin(); it != addedStreams.cend(); ++it )
    {
        std::stringstream ss;

        if(it->second.get_testStream())
        {
            ss << "\"( " << it->second.get_testStreamSrc() << it->second.get_pipeline();
        }
        else
        {
            ss << "\"( -v udpsrc port=" << it->second.get_udpPort() << " ! rtpjitterbuffer"  << it->second.get_pipeline();
        }

        std::string s = ss.str();

        arms::log<arms::LOG_INFO>("Test Stream {}", s);
        threads.push_back(std::thread(&RTSPStream::InitRtspStream, s, it->second.get_tcpPort(), it->second.get_rtspUrl()));
    }

    GSoapInstance gSoapInstance(service_ctx);
    gSoapInstance.runSoapInstance();
//    FOREACH_SERVICE(DECLARE_SERVICE, soap)

//    while( true )
//    {
//        // wait new client
//        if( !soap_valid_socket(soap_accept(soap)) )
//        {
//            arms::log<arms::LOG_DEBUG>("SOAP Valid Socket");
//            soap_stream_fault(soap, std::cerr);
//            return EXIT_FAILURE;
//        }

//        // process service
//        if( soap_begin_serve(soap) )
//        {
//            soap_stream_fault(soap, std::cerr);
//        }
//        FOREACH_SERVICE(DISPATCH_SERVICE, soap)
//        else
//        {
//            arms::log<arms::LOG_DEBUG>("Unknown service");
//        }

//        soap_destroy(soap); // delete managed C++ objects
//        soap_end(soap);     // delete managed memory
//    }


    return EXIT_FAILURE; // Error, normal exit from the main loop only through the signal handler.
}
