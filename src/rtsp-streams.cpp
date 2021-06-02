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
#include <armoury/logger.hpp>


/*
*  Access Functions for configuring streams
*/
bool RTSPStreamConfig::set_pipeline(const char *new_val)
{
    if(!new_val)
    {
        str_err = "pipeline is empty";
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
        str_err = "udpPort is empty";
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
        str_err = "tcpPort is empty";
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
        str_err = "rtspUrl is empty";
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



