/* GStreamer
 * Copyright (C) 2011 Alessandro Decina <alessandro.d@gmail.com>
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
#ifndef _GST_HLS_SINK2_H_
#define _GST_HLS_SINK2_H_

#include "gstm3u8playlist.h"
#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_HLS_SINK2   (gst_hls_sink2_get_type())
#define GST_HLS_SINK2(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_HLS_SINK2,GstHlsSink2))
#define GST_HLS_SINK2_CAST(obj)   ((GstHlsSink2 *) obj)
#define GST_HLS_SINK2_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_HLS_SINK2,GstHlsSink2Class))
#define GST_IS_HLS_SINK2(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_HLS_SINK2))
#define GST_IS_HLS_SINK2_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_HLS_SINK2))

typedef struct _GstHlsSink2 GstHlsSink2;
typedef struct _GstHlsSink2Class GstHlsSink2Class;

typedef enum
{
  MODE_DISK = 0,  //default cache mode, to file
  MODE_MEMORY = 1, //cache media to memory
} GstHlsSink2CacheMode;

typedef struct _HlsFragmentBuf
{
  gchar *location;    // ts filename
  GstMemory * media; // ts fragment
} HlsFragmentBuf;

//[1] property
struct _GstHlsSink2
{
  GstBin bin;

  GstElement *splitmuxsink;
  GstPad *audio_sink, *video_sink;  //hlssink2's sinks

  gchar *location;          //[1] splitmuxsink.location
  gchar *playlist_location; //[1] m3u8 location 
  gchar *playlist_root;     //[1] prefix path to build m3u8 entry-location
  guint playlist_length;    //[1] playlist.window_size
  gint max_files;           //[1] !!!unavailable by now
  gint target_duration;     //[1] splitmuxsink.max-size-time

  GstM3U8Playlist *playlist;
  guint index;  //realtime index of m3u8 entry, update continuously

  gchar *current_location;  //realtime splitmuxsink.location(fragment filename) when new fragment opened
  GstClockTime current_running_time_start;  //realtime running time of first fragment buffer
  GQueue old_locations;  //splitmuxsink.location cache list, in from tail

  //for MODE_MEMORY
  GstHlsSink2CacheMode cache_mode; //[1] save in file or memory
  GstElement *inner_sink;   //retrieve media buffer from When MODE_MEMORY
  gchar* playlist_cache;    //cache playlist content when MODE_MEMORY
  GQueue fragment_cache;    //cache media when MODE_MEMORY, HlsFragmentBuf queue
};

struct _GstHlsSink2Class
{
  GstBinClass bin_class;
};

GType gst_hls_sink2_get_type (void);
gboolean gst_hls_sink2_plugin_init (GstPlugin * plugin);



G_END_DECLS

#endif
