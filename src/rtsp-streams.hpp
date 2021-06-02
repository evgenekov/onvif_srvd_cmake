#ifndef RTSP_STREAMS_HPP
#define RTSP_STREAMS_HPP

#include <stdio.h>
#include <utility>
#include <optional>
#include <map>
#include <armoury/logger.hpp>
#include <armoury/ThreadWarden.hpp>
#include <armoury/json.hpp>

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#define DEFAULT_RTSP_PORT "8554"


class RTSPStreamConfig
{
public:

    RTSPStreamConfig() { clear(); }

    std::string  get_pipeline      (void) const { return pipeline;     }
    std::string  get_udpPort       (void) const { return udpPort;      }
    std::string  get_tcpPort       (void) const { return tcpPort;      }
    std::string  get_rtspUrl       (void) const { return rtspUrl;      }
    bool         get_testStream    (void) const { return testStream;   }
    std::string  get_testStreamSrc (void) const { return testStreamSrc;}

    //methods for parsing opt from cmd
    bool set_pipeline      (const char *new_val);
    bool set_udpPort       (const char *new_val);
    bool set_tcpPort       (const char *new_val);
    bool set_rtspUrl       (const char *new_val);
    bool set_testStream    (int         new_val);
    bool set_testStreamSrc (const char *new_val);


    std::string get_str_err()  const { return str_err;         }
    const char* get_cstr_err() const { return str_err.c_str(); }

    void clear(void);
    bool is_valid(void) const;


private:

    std::string pipeline;
    std::string udpPort;
    std::string tcpPort;
    std::string rtspUrl;
    bool testStream;
    std::string testStreamSrc;

    std::string  str_err;
};


template <typename T>
class GObjWrapper
{
public:
    GObjWrapper(T *gObj)
        : ptr{gObj}
    {
    }

    GObjWrapper() = default;
    
    GObjWrapper(GObjWrapper<T> const &other)
        : ptr{other.ptr}
    {
        ref();
    }
    GObjWrapper &operator=(GObjWrapper<T> const &other)
    {
        unref();
        ptr = other.ptr;
        ref();
        return *this;
    }
    GObjWrapper(GObjWrapper<T> &&other)
        : ptr{std::exchange(other.ptr, nullptr)}
    {
    }
    GObjWrapper &operator=(GObjWrapper<T> &&other)
    {
        unref();
        ptr = std::exchange(other.ptr, nullptr);
        return *this;
    }
    ~GObjWrapper()
    {
        unref();
    }
    
    T *get() const
    {
        return ptr;
    }

private:
    void unref()
    {
        if (ptr)
            g_object_unref(ptr);
        ptr = nullptr;
    }
    void ref()
    {
        if (ptr)
            g_object_ref(ptr);
    }
    T *ptr{};
};

namespace api {

struct StreamSettings : arms::json::Support<StreamSettings>
{
    std::string name{};
    std::string width{"1024"};
    std::string height{"768"};
    std::string url{"rtsp://%s"};
    std::string type{"H264"};
    
private:
    friend class arms::json::Accessor<StreamSettings>;

    template <typename D>
    void toJson(D &dom) const
    {
        addMember(dom, "name", name);
        addMember(dom, "width", width);
        addMember(dom, "height", height);
        addMember(dom, "url", url);
        addMember(dom, "type", type);
    }

    template <typename D>
    void fromJson(D const &dom)
    {
        updateValue(dom, "name", name);
        updateValue(dom, "width", width);
        updateValue(dom, "height", height);
        updateValue(dom, "url", url);
        updateValue(dom, "type", type);
    }
};
} // namespace api


class RTSPStream
{
public:

    bool AddStream(const RTSPStreamConfig& stream);

    std::string get_str_err() const { return str_err;         }
    const char* get_cstr_err()const { return str_err.c_str(); }
    const std::map<std::string, RTSPStreamConfig> &get_streams(void) { return streams; }
    std::optional<RTSPStreamConfig> getStream(std::string streamName) const
    {
        if (streams.count(streamName)){
            return streams.at(streamName);
        }
        return std::nullopt;
    }


private:
    std::map<std::string, RTSPStreamConfig> streams;
    std::string  str_err;
};


class GStreamerRTSPLoop
{
  public:
    static constexpr char const *g_workerName{"getRtspLoop"};
    static constexpr bool g_copyDataOnce{true};
    struct Input
    {
        GMainLoop* loop{nullptr};
    } dataIn;
    struct Output
    {
    } dataOut;
    GStreamerRTSPLoop()
    {
    }
    int work()
    {
        if (dataIn.loop)
        {
            g_main_loop_run(dataIn.loop);
        }
        return 1;
    }

  private:
};


class GStreamerRTSP
{
public:
    GStreamerRTSP(RTSPStreamConfig stream)
    {
        // Build stream URI
        std::stringstream ss;

        if(stream.get_testStream())
        {
            ss << "\"( " << stream.get_testStreamSrc() << stream.get_pipeline();
        }
        else
        {
            ss << "\"( -v udpsrc port=" << stream.get_udpPort() << " ! rtpjitterbuffer"  <<
            stream.get_pipeline();
        }

        std::string s = ss.str();
        arms::log<arms::LOG_INFO>("Test Stream {}", s);

        GError *error = NULL;
        if(!gst_init_check(NULL, NULL, &error))
            arms::log<arms::LOG_INFO>("Didn't set up");
        if(error)
        {
          g_print(error->message, "%s");
          g_clear_error (&error);
        }

        m_loop = g_main_loop_new (NULL, FALSE);

        /* create a server instance */
        server = gst_rtsp_server_new ();
        g_object_set (server.get(), "service", stream.get_tcpPort().c_str(), NULL);

        /* get the mount points for this server, every server has a default object
         * that be used to map uri mount points to media factories */
        mounts = gst_rtsp_server_get_mount_points (server.get());

        /* make a media factory for a test stream. The default media factory can use
         * gst-launch syntax to create pipelines.
         * any launch line works as long as it contains elements named pay%d. Each
         * element with pay%d names will be a stream */
        factory = gst_rtsp_media_factory_new ();
        gst_rtsp_media_factory_set_launch (factory.get(), s.c_str());
        gst_rtsp_media_factory_set_shared (factory.get(), TRUE);

        /* attach the test factory to the /test url */
        gst_rtsp_mount_points_add_factory (mounts.get(), stream.get_rtspUrl().c_str(), factory.get());

        /* attach the server to the default maincontext */
        gst_rtsp_server_attach (server.get(), NULL);

        /* start serving */
        arms::log<arms::LOG_INFO>("stream ready at rtsp://127.0.0.1:{}{}", stream.get_tcpPort(), stream.get_rtspUrl());

        {
        auto [locked] = arms::makeLocked<arms::WriteLock>(m_loopThread.inputData);
        assert(locked);
        locked->loop = m_loop;
        }

        m_loopThread.start();

    }

    ~GStreamerRTSP()
    {
        g_main_loop_quit(m_loop);
        m_loopThread.stop();
    }

private:
    GMainLoop* m_loop;
    GObjWrapper<GstRTSPMediaFactory> factory;
    GObjWrapper<GstRTSPMountPoints> mounts;
    GObjWrapper<GstRTSPServer> server;

    arms::ThreadWarden<GStreamerRTSPLoop> m_loopThread;
};


#endif // RTSP_STREAMS_HPP
