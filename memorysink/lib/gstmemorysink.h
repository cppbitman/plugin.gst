/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wtay@chello.be>
 *
 * gstfilesink.h:
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


#ifndef __GST_MEMORY_SINK_H__
#define __GST_MEMORY_SINK_H__

#include <gst/gst.h>
#include <gst/base/gstbasesink.h>

G_BEGIN_DECLS

#define GST_TYPE_MEMORY_SINK \
  (gst_memory_sink_get_type())
#define GST_MEMORY_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MEMORY_SINK,GstMemorySink))
#define GST_MEMORY_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MEMORY_SINK,GstMemorySinkClass))
#define GST_IS_MEMORY_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MEMORY_SINK))
#define GST_IS_MEMORY_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MEMORY_SINK))
#define GST_MEMORY_SINK_CAST(obj) ((GstMemorySink *)(obj))

typedef struct _GstMemorySink GstMemorySink;
typedef struct _GstMemorySinkClass GstMemorySinkClass;


/**
 * GstMemorySink:
 *
 * Opaque #GstMemorySink structure.
 */
struct _GstMemorySink {
  GstBaseSink parent;

  /*< private >*/
  gchar *location;
  
  gchar  *buffer;
  guint   buffer_size;

  guint64 current_pos;
};

struct _GstMemorySinkClass {
  GstBaseSinkClass parent_class;
};

GType gst_memory_sink_get_type (void);

G_END_DECLS

#endif /* __GST_FILE_SINK_H__ */
