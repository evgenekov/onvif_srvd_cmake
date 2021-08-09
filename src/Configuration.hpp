#pragma once

#include "ConfigLoader.hpp"
#include "ServiceContext.h"
#include "eth_dev_param.h"
#include <map>
#include <optional>
#include <string>

struct Scopes
{
    int scopes_id{0};
    std::string scopeUri{};

    Scopes() = default;
    Scopes(libconfig::Setting const &wf)
    {
        if (wf.isGroup() && wf.lookupValue("scopes_id", scopes_id) && wf.lookupValue("scopeUri", scopeUri))
        {
            return;
        }
        throw std::runtime_error("scopes config parse error");
    }
    Scopes(int const id) : scopes_id{id}
    {
        switch (id)
        {
        case 0:
            scopeUri = "onvif://www.onvif.org/name/Combiner-Feed";
            break;
        case 1:
            scopeUri = "onvif://www.onvif.org/location/[PLATFORM ID]";
            break;
        case 2:
            scopeUri = "onvif://www.onvif.org/Profile/Streaming";
            break;
        case 3:
            scopeUri = "onvif://www.onvif.org/Profile/S";
            break;
        }
    }
};

struct Profiles
{
    int profile_id{0};
    std::string name{};
    std::string width{};
    std::string height{};
    std::string url{};
    std::string snapUrl{};
    std::string type{};

    Profiles() = default;
    Profiles(libconfig::Setting const &wf)
    {
        if (wf.isGroup() && wf.lookupValue("profile_id", profile_id) && wf.lookupValue("name", name) &&
            wf.lookupValue("width", width) && wf.lookupValue("height", height) && wf.lookupValue("url", url) &&
            wf.lookupValue("snapUrl", snapUrl) && wf.lookupValue("type", type))
        {
            return;
        }
        throw std::runtime_error("waveform config parse error");
    }
    Profiles(int const id) : profile_id{id}
    {
        switch (id)
        {
        case 0:
            name = "Right_Monitor";
            width = "1024";
            height = "768";
            url = "rtsp://%s:554/right";
            snapUrl = "";
            type = "H264";
            break;
        case 1:
            name = "Left_Monitor";
            width = "1024";
            height = "768";
            url = "rtsp://%s:8554/left";
            snapUrl = "";
            type = "H264";
            break;
        }
    }
};

struct RTSPStreams
{
    int rtspstream_id{0};
    std::string pipeline{};
    std::string udpPort{};
    std::string tcpPort{};
    std::string rtspUrl{};
    bool testStream{};
    std::string testStreamSrc{};

    RTSPStreams() = default;
    RTSPStreams(libconfig::Setting const &wf)
    {
        if (wf.isGroup() && wf.lookupValue("rtspstream_id", rtspstream_id) && wf.lookupValue("pipeline", pipeline) &&
            wf.lookupValue("udpPort", udpPort) && wf.lookupValue("tcpPort", tcpPort) &&
            wf.lookupValue("rtspUrl", rtspUrl) && wf.lookupValue("testStream", testStream) &&
            wf.lookupValue("testStreamSrc", testStreamSrc))
        {
            return;
        }
        throw std::runtime_error("waveform config parse error");
    }
    RTSPStreams(int const id) : rtspstream_id{id}
    {
        switch (id)
        {
        case 0:
            pipeline = " ! application/x-rtp,clock-rate=90000,payload=96 ! rtph264depay ! rtph264pay name=pay0 pt=96 )\"";
            udpPort = "5001";
            tcpPort = "8554";
            rtspUrl = "/left";
            testStream = false;
            testStreamSrc = "videotestsrc pattern=ball";
            break;
        case 1:
            pipeline = " ! application/x-rtp,clock-rate=90000,payload=96 ! rtph264depay ! rtph264pay name=pay0 pt=96 )\"";
            udpPort = "5000";
            tcpPort = "554";
            rtspUrl = "/right";
            testStream = false;
            testStreamSrc = "videotestsrc";
            break;
        }
    }
};

struct Configuration
{
    Configuration() = default;
    explicit Configuration(std::optional<std::string> const &configFile);
    explicit Configuration(ConfigLoader &loader);
    void loadAllSettings(ConfigLoader &loader);

    void SetServiceContext(ServiceContext *service_ctx);

    // Daemon Info
    const char *pid_file{"/tmp/onvif_svrd_debug.pid"};
    const char *logLevel{"debug"};
    const char *logFile{""};
    int logFileSizeMb{0};
    int logFileCount{0};
    bool logAsync{false};

    // ONVIF Service Options
    int port{1000};
    std::string user{"admin"};
    std::string password{"admin"};
    std::string manufacturer{"Rinicom"};
    std::string model{"Watchman"};
    std::string firmware_version{"UNKNOWN"};
    std::string serial_number{"UNKNOWN"};
    std::string hardware_id{"UNKNOWN"};
    std::string interfaces{"enp5s0"};
    std::string tz_format{"0"};

    std::vector<Scopes> scopes{Scopes{0}, Scopes{1}, Scopes{2}, Scopes{3}};
    std::vector<Profiles> profiles{Profiles{0}, Profiles{1}};
    std::vector<RTSPStreams> rtspStreams{RTSPStreams{0}, RTSPStreams{1}};

    std::vector<Eth_Dev_Param> eth_ifs;
};
