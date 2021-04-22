#ifndef RTSP_STREAMS_HPP
#define RTSP_STREAMS_HPP

#include <stdio.h>
#include <utility>
#include <thread>
#include <map>
#include "logger.hpp"

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#define DEFAULT_RTSP_PORT "8554"


class RTSPStreamConfig
{
public:

    RTSPStreamConfig() { clear(); }

    std::string  get_pipeline   (void) const { return pipeline;   }
    std::string  get_udpPort    (void) const { return udpPort;    }
    std::string  get_tcpPort    (void) const { return tcpPort;    }
    std::string  get_rtspUrl    (void) const { return rtspUrl;    }
    bool         get_testStream (void) const { return testStream; }

    //methods for parsing opt from cmd
    bool set_pipeline   (const char *new_val);
    bool set_udpPort    (const char *new_val);
    bool set_tcpPort    (const char *new_val);
    bool set_rtspUrl    (const char *new_val);
    bool set_testStream (const char *new_val);


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
    }
    void ref()
    {
        if (ptr)
            g_object_ref(ptr);
    }
    T *ptr{};
};

class RTSPStream
{
public:
    static void InitRtspStream(std::string pipelineStr, std::string portStr, std::string uriStr);

    std::string get_str_err() const { return str_err;         }
    const char* get_cstr_err()const { return str_err.c_str(); }

    const std::map<std::string, RTSPStreamConfig> &get_streams(void) { return streams; }
    bool AddStream(const RTSPStreamConfig& stream);

private:
    std::map<std::string, RTSPStreamConfig> streams;
    std::string  str_err;
};


#endif // RTSP_STREAMS_HPP
