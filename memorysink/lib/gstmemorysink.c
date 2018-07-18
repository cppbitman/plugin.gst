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

#include "gstmemorysink.h"

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

GST_DEBUG_CATEGORY_STATIC (gst_memory_sink_debug);
#define GST_CAT_DEFAULT gst_memory_sink_debug


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
 
#define gst_memory_sink_parent_class parent_class
G_DEFINE_TYPE (GstMemorySink, gst_memory_sink, GST_TYPE_BASE_SINK);

static void
gst_memory_sink_class_init (GstMemorySinkClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
  GstBaseSinkClass *gstbasesink_class = GST_BASE_SINK_CLASS (klass);

  gobject_class->dispose = gst_memory_sink_dispose;

  gobject_class->set_property = gst_memory_sink_set_property;
  gobject_class->get_property = gst_memory_sink_get_property;

  gst_element_class_set_static_metadata (gstelement_class,
      "Memory Sink",
      "Sink/Memory", "Write stream to memory",
      "Sevent <daihongjun at kedacom dot com>");
  gst_element_class_add_static_pad_template (gstelement_class, &sinktemplate);

  gstbasesink_class->start = GST_DEBUG_FUNCPTR (gst_memory_sink_start);
  gstbasesink_class->stop = GST_DEBUG_FUNCPTR (gst_memory_sink_stop);
  gstbasesink_class->query = GST_DEBUG_FUNCPTR (gst_memory_sink_query);
  gstbasesink_class->render = GST_DEBUG_FUNCPTR (gst_memory_sink_render);
  gstbasesink_class->render_list =
      GST_DEBUG_FUNCPTR (gst_memory_sink_render_list);
  gstbasesink_class->event = GST_DEBUG_FUNCPTR (gst_memory_sink_event);
}

static void
gst_memory_sink_init (GstMemorySink * memorysink)
{
  GST_DEBUG_CATEGORY_INIT (gst_memory_sink_debug, "memorysink", 0, "memorysink element");
  gst_base_sink_set_sync (GST_BASE_SINK (memorysink), FALSE);
}

static void
gst_memory_sink_dispose (GObject * object)
{
  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gst_memory_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstMemorySink *sink = GST_MEMORY_SINK (object);

  switch (prop_id) {
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
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_memory_sink_query (GstBaseSink * bsink, GstQuery * query)
{
  gboolean res;

  switch (GST_QUERY_TYPE (query)) {
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
  return GST_BASE_SINK_CLASS (parent_class)->event (sink, event);
}

static GstFlowReturn
gst_memory_sink_render_list (GstBaseSink * bsink, GstBufferList * buffer_list)
{
  goto no_data;

no_data:
  {
    GST_LOG_OBJECT (bsink, "empty buffer list");
    return GST_FLOW_OK;
  }
}

static GstFlowReturn
gst_memory_sink_render (GstBaseSink * sink, GstBuffer * buffer)
{
  GstFlowReturn flow = GST_FLOW_OK;

  return flow;
}

static gboolean
gst_memory_sink_start (GstBaseSink * basesink)
{
  return TRUE;
}

static gboolean
gst_memory_sink_stop (GstBaseSink * basesink)
{
  return TRUE;
}