/* GStreamer
 * Copyright (C) 2008 Wim Taymans <wim.taymans at gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <sstream>

#include "rtsp-streams.hpp"
#include "logger.hpp"


/*
*  Access Functions for configuring streams
*/
bool RTSPStreamConfig::set_pipeline(const char *new_val)
{
    if(!new_val)
    {
        str_err = "Name is empty";
        return false;
    }


    pipeline = new_val;
    return true;
}


/*
*  Access Functions for configuring streams
*/
bool RTSPStreamConfig::set_udpPort(const char *new_val)
{
    if(!new_val)
    {
        str_err = "Name is empty";
        return false;
    }


    udpPort = new_val;
    return true;
}


/*
*  Access Functions for configuring streams
*/
bool RTSPStreamConfig::set_tcpPort(const char *new_val)
{
    if(!new_val)
    {
        str_err = "Name is empty";
        return false;
    }


    tcpPort = new_val;
    return true;
}


/*
*  Access Functions for configuring streams
*/
bool RTSPStreamConfig::set_rtspUrl(const char *new_val)
{
    if(!new_val)
    {
        str_err = "Name is empty";
        return false;
    }


    rtspUrl = new_val;
    return true;
}


/*
*  Access Functions for configuring streams
*/
bool RTSPStreamConfig::set_testStream(int new_val)
{
    testStream = new_val;
    return true;
}


/*
*  Access Functions for configuring streams
*/
bool RTSPStreamConfig::set_testStreamSrc(const char *new_val)
{
    if(!new_val)
    {
        str_err = "testStreamSrc is empty";
        return false;
    }


    testStreamSrc = new_val;
    return true;
}


/*
*  Access Functions for configuring streams
*/
void RTSPStreamConfig::clear()
{
    pipeline.clear();
    udpPort.clear();
    tcpPort.clear();
    rtspUrl.clear();
    testStream = 0;
}


/*
*  Access Functions for configuring streams
*/
bool RTSPStreamConfig::is_valid() const
{
    return ( !pipeline.empty()  &&
             !udpPort.empty()   &&
             !tcpPort.empty()   &&
             !rtspUrl.empty()    );
}


bool RTSPStream::AddStream(const RTSPStreamConfig& stream)
{
    if( !stream.is_valid() )
    {
        str_err = "stream has unset parameters";
        return false;
    }


    if( streams.find(stream.get_rtspUrl()) != streams.end() )
    {
        str_err = "stream: " + stream.get_rtspUrl() +  " already exists";
        return false;
    }


    streams[stream.get_rtspUrl()] = stream;
    return true;
}


void RTSPStream::InitRtspStream(std::string pipelineStr, std::string portStr, std::string uriStr)
{
  // Convert std::strings to const char* for C compatibility
  const char* pipeline = pipelineStr.c_str();
  const char* port = portStr.c_str();
  const char* uri = uriStr.c_str();

  GError *error = NULL;
  if(!gst_init_check(NULL, NULL, &error))
      arms::log<arms::LOG_INFO>("Didn't set up");
  if(error)
  {
    g_print(error->message, "%s");
    g_clear_error (&error);
  }

  GObjWrapper<GMainLoop> loop = g_main_loop_new (NULL, FALSE);

  /* create a server instance */
  GObjWrapper<GstRTSPServer> server = gst_rtsp_server_new ();
  //printf("\n%d\n", gst_rtsp_server_get_backlog(server));
  g_object_set (server.get(), "service", port, NULL);

  /* get the mount points for this server, every server has a default object
   * that be used to map uri mount points to media factories */
  GObjWrapper<GstRTSPMountPoints> mounts = gst_rtsp_server_get_mount_points (server.get());

  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines.
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */
  GObjWrapper<GstRTSPMediaFactory> factory {gst_rtsp_media_factory_new ()};
  gst_rtsp_media_factory_set_launch (factory.get(), pipeline);
  gst_rtsp_media_factory_set_shared (factory.get(), TRUE);

  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory (mounts.get(), uri, factory.get());

  /* don't need the ref to the mapper anymore */
  g_object_unref (mounts.get());

  /* attach the server to the default maincontext */
  gst_rtsp_server_attach (server.get(), NULL);

  /* start serving */
  arms::log<arms::LOG_INFO>("stream ready at rtsp://127.0.0.1:{}{}", port, uri);
  g_main_loop_run (loop.get());
}

