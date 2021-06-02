#ifndef RTSP_STREAMS_HPP
#define RTSP_STREAMS_HPP

#include <stdio.h>
#include <utility>
#include <optional>
#include <map>
#include <armoury/logger.hpp>
#include <armoury/ThreadWarden.hpp>
#include <armoury/json.hpp>
#include <Configuration.hpp>

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#define DEFAULT_RTSP_PORT "8554"


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
    GStreamerRTSP(RTSPStreams stream)
    {
        // Build stream URI
        std::stringstream ss;

        if(stream.testStream)
        {
            ss << "\"( " << stream.testStreamSrc << stream.pipeline;
        }
        else
        {
            ss << "\"( -v udpsrc port=" << stream.udpPort << " ! rtpjitterbuffer"  <<
            stream.pipeline;
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
        g_object_set (server.get(), "service", stream.tcpPort.c_str(), NULL);

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
        gst_rtsp_mount_points_add_factory (mounts.get(), stream.rtspUrl.c_str(), factory.get());

        /* attach the server to the default maincontext */
        gst_rtsp_server_attach (server.get(), NULL);

        /* start serving */
        arms::log<arms::LOG_INFO>("stream ready at rtsp://127.0.0.1:{}{}", stream.tcpPort, stream.rtspUrl);

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
