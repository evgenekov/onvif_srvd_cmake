#ifndef RTSP_STREAMS_HPP
#define RTSP_STREAMS_HPP

#include <stdio.h>
#include <utility>
#include <thread>

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
    void InitRtspStream(const char* pipeline, const char* port, const char* uri);
};



#endif // RTSP_STREAMS_HPP
