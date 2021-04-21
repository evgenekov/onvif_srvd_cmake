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

#include "rtsp-streams.hpp"

//static char *port = (char *) DEFAULT_RTSP_PORT;



void RTSPStream::InitRtspStream(const char* pipeline, const char* port, const char* uri)
{
    static GOptionEntry entries[] = {
    {"port", 'p', 0, G_OPTION_ARG_STRING, &port,
        "Port to listen on (default: " DEFAULT_RTSP_PORT ")", "PORT"},
    {NULL}
    };    
    
  GError *error = NULL;
  if(!gst_init_check(NULL, NULL, &error))
      g_print("Didn't set up");
  if(error)
  {
    g_print(error->message, "%s");
    g_clear_error (&error);
  }
  
  GObjWrapper<GOptionContext> optctx = g_option_context_new ("TEST");
  g_option_context_add_main_entries (optctx.get(), entries, NULL);
  g_option_context_add_group (optctx.get(), gst_init_get_option_group ());
  g_option_context_free (optctx.get());
  
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
  g_print ("\nstream ready at rtsp://127.0.0.1:%s%s", port, uri);
  g_main_loop_run (loop.get());
    
}

