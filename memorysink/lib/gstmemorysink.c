/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wtay@chello.be>
 *                    2006 Wim Taymans <wim@fluendo.com>
 *
 * gstfilesink.c:
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
/**
 * SECTION:element-filesink
 * @title: filesink
 * @see_also: #GstFileSrc
 *
 * Write incoming data to a file in the local file system.
 *
 * ## Example launch line
 * |[
 * gst-launch-1.0 v4l2src num-buffers=1 ! jpegenc ! filesink location=capture1.jpeg
 * ]| Capture one frame from a v4l2 camera and save as jpeg image.
 *
 */

#include "../gst/gst-i18n-lib.h"

#include "gstmemorysink.h"

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

GST_DEBUG_CATEGORY_STATIC (gst_memory_sink_debug);
#define GST_CAT_DEFAULT gst_memory_sink_debug

#define DEFAULT_LOCATION NULL
#define DEFAULT_BUFFER_SIZE 	8*1024 * 1024

enum {
  PROP_0,
  PROP_LOCATION,
  PROP_BUFFER_SIZE,
  PROP_LAST
};

enum {
  SIGNAL_MOVE,
  SIGNAL_LAST
};

static guint signals[SIGNAL_LAST];

static void gst_memory_sink_dispose (GObject * object);

static void gst_memory_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_memory_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_memory_sink_start (GstBaseSink * sink);
static gboolean gst_memory_sink_stop (GstBaseSink * sink);
static gboolean gst_memory_sink_event (GstBaseSink * sink, GstEvent * event);
static GstFlowReturn gst_memory_sink_render (GstBaseSink * sink,
    GstBuffer * buffer);
static GstFlowReturn gst_memory_sink_render_list (GstBaseSink * sink,
    GstBufferList * list);
static gboolean gst_memory_sink_query (GstBaseSink * bsink, GstQuery * query);

static GstMemory* gst_memory_sink_move ( GstMemorySink* sink, gchar* cur_location);

#define _do_init \
  GST_DEBUG_CATEGORY_INIT (gst_memory_sink_debug, "memorysink", 0, "memorysink element");

#define gst_memory_sink_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstMemorySink, gst_memory_sink, GST_TYPE_BASE_SINK,
    _do_init);

static void
gst_memory_sink_class_init (GstMemorySinkClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
  GstBaseSinkClass *gstbasesink_class = GST_BASE_SINK_CLASS (klass);

  gst_element_class_set_static_metadata (gstelement_class, "Memory Sink",
      "Sink/Memory", "Write stream to memory", "daihongjun <daihongjun at kedacom dot com>");

  gst_element_class_add_static_pad_template (gstelement_class, &sinktemplate);

  gobject_class->dispose = gst_memory_sink_dispose;

  gobject_class->set_property = gst_memory_sink_set_property;
  gobject_class->get_property = gst_memory_sink_get_property;
  
  gstbasesink_class->start = GST_DEBUG_FUNCPTR (gst_memory_sink_start);
  gstbasesink_class->stop = GST_DEBUG_FUNCPTR (gst_memory_sink_stop);
  gstbasesink_class->query = GST_DEBUG_FUNCPTR (gst_memory_sink_query);
  gstbasesink_class->render = GST_DEBUG_FUNCPTR (gst_memory_sink_render);
  gstbasesink_class->render_list = GST_DEBUG_FUNCPTR (gst_memory_sink_render_list);
  gstbasesink_class->event = GST_DEBUG_FUNCPTR (gst_memory_sink_event);
  
  g_object_class_install_property (gobject_class, PROP_LOCATION,
      g_param_spec_string ("location", "File Location",
          "Location of the file to write", DEFAULT_LOCATION,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));    
          
  g_object_class_install_property (gobject_class, PROP_BUFFER_SIZE,
      g_param_spec_uint ("buffer-size", "Buffering size",
          "Size of buffer in number of bytes for line or full buffer-mode", 0,
          G_MAXUINT, DEFAULT_BUFFER_SIZE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
          
  /**
   * GstMemorySink::move:
   * @memorysink: the #GstMemorySink
   *
   * When called by the user, this action signal moves and returns the cached media buffer as GstMemory .
   *
   */
  
  signals[SIGNAL_MOVE] =
      g_signal_new ("move", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstMemorySinkClass,
          move), NULL, NULL, NULL, GST_TYPE_MEMORY, 1, G_TYPE_STRING);
  klass->move = gst_memory_sink_move;
}

static void
gst_memory_sink_init (GstMemorySink * sink)
{
  sink->location = DEFAULT_LOCATION;
  sink->buffer_size = DEFAULT_BUFFER_SIZE;
  sink->buffer = NULL;
  sink->current_pos = 0;
  sink->eos = FALSE;
  gst_base_sink_set_sync (GST_BASE_SINK (sink), FALSE);
}

static void
gst_memory_sink_dispose (GObject * object)
{
  GstMemorySink *sink = GST_MEMORY_SINK (object);

  G_OBJECT_CLASS (parent_class)->dispose (object);

  if(sink->location) {
    g_free (sink->location);
    sink->location = NULL;
  }
  if(sink->buffer) {
    g_free (sink->buffer);
    sink->buffer = NULL;
  }
  sink->current_pos = 0;
  sink->eos = FALSE;
}

static gboolean
gst_memory_sink_set_location (GstMemorySink * sink, const gchar * location,
    GError ** error)
{
  if (sink->buffer)//null after move
    goto was_open;
  
  if(sink->location)
    g_free (sink->location);
  
  if (location != NULL) {
    /* we store the filename as we received it from the application. On Windows
     * this should be in UTF8 */
    sink->location = g_strdup (location);
    GST_INFO_OBJECT (sink, "location : %s", sink->location);
  } else {
    sink->location = NULL;
  }

  return TRUE;

  /* ERRORS */
was_open:
  {
    g_warning ("Changing the `location' property on memorysink when a memory is "
        "allocated is not supported.");
    g_set_error (error, GST_URI_ERROR, GST_URI_ERROR_BAD_STATE,
        "Changing the 'location' property on memorysink when a memory is "
        "allocated is not supported");
    return FALSE;
  }
}

static void
gst_memory_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstMemorySink *sink = GST_MEMORY_SINK (object);

  switch (prop_id) {
    case PROP_LOCATION:
      gst_memory_sink_set_location (sink, g_value_get_string (value), NULL);
      break;
    case PROP_BUFFER_SIZE:
      sink->buffer_size = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_memory_sink_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstMemorySink *sink = GST_MEMORY_SINK (object);

  switch (prop_id) {
    case PROP_LOCATION:
      g_value_set_string (value, sink->location);
      break;
    case PROP_BUFFER_SIZE:
      g_value_set_uint (value, sink->buffer_size);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_memory_sink_query (GstBaseSink * bsink, GstQuery * query)
{
  gboolean res;
  GstMemorySink *self;
  GstFormat format;

  self = GST_MEMORY_SINK (bsink);

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_POSITION:
      gst_query_parse_position (query, &format, NULL);

      switch( format ) {
        case GST_FORMAT_DEFAULT:
        case GST_FORMAT_BYTES:
          gst_query_set_position (query, GST_FORMAT_BYTES, self->current_pos);
          res = TRUE;
          break;
        default:
          res = FALSE;
          break;
      }

      break;

    case GST_QUERY_FORMATS:
      gst_query_set_formats (query, 2, GST_FORMAT_DEFAULT, GST_FORMAT_BYTES);
      res = TRUE;
      break;

    default:
      res = GST_BASE_SINK_CLASS (parent_class)->query (bsink, query);
      break;
  }
  return res;
}

/* handle events (search) */
static gboolean
gst_memory_sink_event (GstBaseSink * sink, GstEvent * event)
{
  GstEventType type;
  GstMemorySink *msink;
  
  msink = GST_MEMORY_SINK (sink);
  switch (type) {
    case GST_EVENT_EOS:
      msink->eos = TRUE;
      break;
    default:
      break;
  }

  return GST_BASE_SINK_CLASS (parent_class)->event (sink, event);
}

static GstFlowReturn
gst_memory_sink_copy_buffers(GstMemorySink * sink, GstBuffer ** buffers,
    guint num_buffers, guint8 * mem_nums, guint total_mems, guint64 * current_pos)
{
  gchar *pcurrent = sink->buffer + *current_pos;
  
  GstMemory *mem;
  GstMapInfo *map_infos;

  GstFlowReturn flow_ret;
  guint i, j, k;

  GST_LOG_OBJECT (sink, "%u buffers, %u memories", num_buffers, total_mems);

  map_infos = g_newa (GstMapInfo, total_mems);
  for(i=0, k = 0; i<num_buffers; ++i) {

    g_assert( mem_nums[i]== gst_buffer_n_memory(buffers[i]) );
    //copy memories of ith GstBuffer
    for(j=0; j<mem_nums[i]; ++j, ++k ) {
      mem = gst_buffer_peek_memory(buffers[i], j);
      if( gst_memory_map(mem, &map_infos[k], GST_MAP_READ) ) {
        if(map_infos[k].size > 0) {
          if( pcurrent-sink->buffer+map_infos[k].size > sink->buffer_size ) {
            goto write_error;
          }
          memcpy( pcurrent, map_infos[k].data, map_infos[k].size );
          pcurrent += map_infos[k].size;
        }
      }
    }

  }

  g_assert( k == total_mems );
  *current_pos += (pcurrent - (sink->buffer + *current_pos) );

  flow_ret = GST_FLOW_OK;
  
out:
  for (i = 0; i < total_mems; ++i)
    gst_memory_unmap (map_infos[i].memory, &map_infos[i]);

  return flow_ret;

write_error:
  {
    GST_ELEMENT_ERROR (sink, RESOURCE, NO_SPACE_LEFT, (_("No enough buffer space for writing GstBuffer.")), (NULL));
    flow_ret = GST_FLOW_ERROR;
    goto out;
  }
}

static GstFlowReturn
gst_memory_sink_render_buffers (GstMemorySink * sink, GstBuffer ** buffers,
    guint num_buffers, guint8 * mem_nums, guint total_mems)
{
  GST_DEBUG_OBJECT (sink,
    "writing %u buffers (%u memories) at position %" G_GUINT64_FORMAT,
    num_buffers, total_mems, sink->current_pos);

  return gst_memory_sink_copy_buffers(sink, buffers, num_buffers, mem_nums, total_mems, &sink->current_pos);
}

static GstFlowReturn
gst_memory_sink_render_list (GstBaseSink * bsink, GstBufferList * buffer_list)
{
  GstMemorySink *sink;

  GstBuffer **buffers;
  guint8 *mem_nums;
  guint num_buffers;
  guint total_mems;

  guint i;
  GstFlowReturn flow;

  sink = GST_MEMORY_SINK_CAST (bsink);
  num_buffers = gst_buffer_list_length (buffer_list);
  
  if (num_buffers == 0)
    goto no_data;

  buffers = g_newa (GstBuffer *, num_buffers);
  mem_nums = g_newa (guint8, num_buffers);

  for (i = 0, total_mems = 0; i < num_buffers; ++i) {
    buffers[i] = gst_buffer_list_get (buffer_list, i);
    mem_nums[i] = gst_buffer_n_memory (buffers[i]);
    total_mems += mem_nums[i];
  }

  flow = gst_memory_sink_render_buffers (sink, buffers, num_buffers, mem_nums, total_mems);

no_data:
  {
    GST_LOG_OBJECT (bsink, "empty buffer list");
    return GST_FLOW_OK;
  }
}

static GstFlowReturn
gst_memory_sink_render (GstBaseSink * sink, GstBuffer * buffer)
{
  GstMemorySink *memorysink;
  GstFlowReturn flow;
  guint8 n_mem;

  memorysink = GST_MEMORY_SINK_CAST (sink);

  n_mem = gst_buffer_n_memory (buffer);

  if (n_mem > 0)
    flow = gst_memory_sink_render_buffers (memorysink, &buffer, 1, &n_mem, n_mem);
  else
    flow = GST_FLOW_OK;

  return flow;
}

static gboolean
gst_memory_sink_alloc_memory (GstMemorySink * sink)
{
  if(sink->location == NULL || sink->location[0] == '\0')
    goto no_location;

  sink->buffer = g_malloc0(sink->buffer_size);
  if( sink->buffer == NULL )
    goto open_failed;

  sink->current_pos = 0;

  GST_DEBUG_OBJECT (sink, "opened memory for location %s", sink->location);

  return TRUE;
  /* ERRORS */
no_location:
  {
    GST_ELEMENT_ERROR (sink, RESOURCE, NOT_FOUND,
        (_("No location specified for writing.")), (NULL));
    return FALSE;
  }

open_failed:
  {
    GST_ELEMENT_ERROR (sink, RESOURCE, OPEN_WRITE,
        (_("Could not open memory for writing \"%s\"."), sink->location),
        GST_ERROR_SYSTEM);
    return FALSE;
  }
}

static void
gst_memory_sink_free_memory (GstMemorySink * sink)
{
  if( sink->buffer )
  {
    g_free (sink->buffer);
    sink->buffer = NULL;

    GST_DEBUG_OBJECT (sink, "closed memory");
  }
}

static gboolean
gst_memory_sink_start (GstBaseSink * basesink)
{
  return gst_memory_sink_alloc_memory( GST_MEMORY_SINK(basesink) );
}

static gboolean
gst_memory_sink_stop (GstBaseSink * basesink)
{
  gst_memory_sink_free_memory( GST_MEMORY_SINK(basesink) );
  return TRUE;
}

static GstMemory*
gst_memory_sink_move ( GstMemorySink* sink, gchar* cur_location)
{
  g_return_val_if_fail(cur_location != NULL, NULL);
  g_return_val_if_fail( g_strcmp0(cur_location, sink->location) == 0 , NULL);
  g_warn_if_fail(sink->eos);
  GST_DEBUG_OBJECT(sink, "Before move end-of-stream : %s", sink->eos ? "TRUE":"FALSE");
  

  GstMemory *media = gst_memory_new_wrapped(
    GST_MEMORY_FLAG_READONLY|GST_MEMORY_FLAG_PHYSICALLY_CONTIGUOUS,
    sink->buffer,  DEFAULT_BUFFER_SIZE, 0, sink->current_pos, NULL, NULL);

  g_return_val_if_fail(media != NULL, NULL);

  sink->buffer = NULL;
  sink->current_pos = 0;
  
  return media;
}