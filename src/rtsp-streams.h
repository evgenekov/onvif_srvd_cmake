#include <stdio.h>

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#define DEFAULT_RTSP_PORT "554"

int InitRtspStream(const char* pipeline);
