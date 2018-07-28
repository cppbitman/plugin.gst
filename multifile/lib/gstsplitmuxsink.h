/* GStreamer split muxer bin
 * Copyright (C) 2014 Jan Schmidt <jan@centricular.com>
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

#ifndef __GST_SPLITMUXSINK_H__
#define __GST_SPLITMUXSINK_H__

#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

G_BEGIN_DECLS
#define GST_TYPE_SPLITMUX_SINK               (gst_splitmux_sink_get_type())
#define GST_SPLITMUX_SINK(obj)               (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_SPLITMUX_SINK,GstSplitMuxSink))
#define GST_SPLITMUX_SINK_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_SPLITMUX_SINK,GstSplitMuxSinkClass))
#define GST_IS_SPLITMUX_SINK(obj)            (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_SPLITMUX_SINK))
#define GST_IS_SPLITMUX_SINK_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_SPLITMUX_SINK))
typedef struct _GstSplitMuxSink GstSplitMuxSink;
typedef struct _GstSplitMuxSinkClass GstSplitMuxSinkClass;

GType gst_splitmux_sink_get_type (void);
gboolean register_splitmuxsink (GstPlugin * plugin);

typedef enum _SplitMuxInputState
{
  SPLITMUX_INPUT_STATE_STOPPED,
  SPLITMUX_INPUT_STATE_COLLECTING_GOP_START,    /* Waiting for the next ref ctx keyframe */
  SPLITMUX_INPUT_STATE_WAITING_GOP_COLLECT,     /* Waiting for all streams to collect GOP */
  SPLITMUX_INPUT_STATE_FINISHING_UP             /* Got EOS from reference ctx, send everything */
} SplitMuxInputState;

typedef enum _SplitMuxOutputState
{
  SPLITMUX_OUTPUT_STATE_STOPPED,
  SPLITMUX_OUTPUT_STATE_AWAITING_COMMAND,       /* Waiting first command packet from input */
  SPLITMUX_OUTPUT_STATE_OUTPUT_GOP,     /* Outputting a collected GOP */
  SPLITMUX_OUTPUT_STATE_ENDING_FILE,    /* Finishing the current fragment */
  SPLITMUX_OUTPUT_STATE_START_NEXT_FILE /* Restarting after ENDING_FILE */
} SplitMuxOutputState;

typedef struct _SplitMuxOutputCommand
{
  gboolean start_new_fragment;  /* Whether to start a new fragment before advancing output ts */
  GstClockTimeDiff max_output_ts;       /* Set the limit to stop GOP output */
} SplitMuxOutputCommand;

typedef struct _MqStreamBuf//info of GstBuffer streaming into _GstSplitMuxSink.queue
{
  gboolean keyframe;
  GstClockTimeDiff run_ts;
  guint64 buf_size;
  GstClockTime duration;
} MqStreamBuf;

typedef struct _MqStreamCtx
{
  gint refcount;

  GstSplitMuxSink *splitmux;

  guint q_overrun_id;     //_GstSplitMuxSink.queue.overrun_id
  guint sink_pad_block_id; //_GstSplitMuxSink.queue.sinkpad.probe_id.(DATA_DOWNSTREAM+EVENT_FLUSH+QUERY_DOWNSTREAM)
  guint src_pad_block_id; //_GstSplitMuxSink.queue.srcpad.probe_id.(DATA_DOWNSTREAM+EVENT_FLUSH)

  gboolean is_reference;  //_GstSplitMuxSink.reference_ctx

  gboolean flushing;
  gboolean in_eos;
  gboolean out_eos;
  gboolean need_unblock;
  gboolean caps_change;

  GstSegment in_segment;
  GstSegment out_segment;

  GstClockTimeDiff in_running_time;
  GstClockTimeDiff out_running_time;

  GstBuffer *prev_in_keyframe; /* store keyframe for each GOP */

  GstElement *q;    //_GstSplitMuxSink.queue
  GQueue queued_bufs;

  GstPad *sinkpad;  //_GstSplitMuxSink.queue.sinkpad
  GstPad *srcpad;   //_GstSplitMuxSink.queue.srcpad

  GstBuffer *cur_out_buffer;
  GstEvent *pending_gap;
} MqStreamCtx;

//[1] properties

struct _GstSplitMuxSink
{
  GstBin parent;

  GMutex lock;
  GCond input_cond;
  GCond output_cond;

  gdouble mux_overhead;    //[1]

  GstClockTime threshold_time;    //[1]
  guint64 threshold_bytes;    //[1]
  guint max_files;    //[1]
  gboolean send_keyframe_requests;    //[1]
  gchar *threshold_timecode_str;    //[1]
  GstClockTime next_max_tc_time;
  GstClockTime alignment_threshold;    //[1]

  GstElement *muxer;    //[2] pointer to muxer added to splitmuxsink, not increasing reference-count
  GstElement *sink;     //[2] pointer to active_sink or internal-sink element of active_sink

  GstElement *provided_muxer;   //[1]

  GstElement *provided_sink;    //[1]
  GstElement *active_sink;

  gboolean ready_for_output;

  gchar *location;    //[1]
  guint fragment_id;  //fragment sequence number

  GList *contexts;    //MqStreamCtx

  SplitMuxInputState input_state;
  GstClockTimeDiff max_in_running_time;
  /* Number of bytes sent to the
   * current fragment */
  guint64 fragment_total_bytes;
  /* Number of bytes we've collected into
   * the GOP that's being collected */
  guint64 gop_total_bytes;
  /* Start time of the current fragment */
  GstClockTimeDiff fragment_start_time;
  /* Start time of the current GOP */
  GstClockTimeDiff gop_start_time;

  GQueue out_cmd_q;             /* Queue of commands for output thread */

  SplitMuxOutputState output_state;
  GstClockTimeDiff max_out_running_time;

  guint64 muxed_out_bytes;

  MqStreamCtx *reference_ctx;  //video pad MqStreamCtx associated with internal queue
  /* Count of queued keyframes in the reference ctx */
  guint queued_keyframes;

  gboolean switching_fragment;

  gboolean have_video;    //video pad requested

  gboolean need_async_start;
  gboolean async_pending;

  gboolean use_robust_muxing;    //[1]
  gboolean muxer_has_reserved_props;

  gboolean split_now;
};

struct _GstSplitMuxSinkClass
{
  GstBinClass parent_class;

  /* actions */
  void     (*split_now)   (GstSplitMuxSink * splitmux);
};

G_END_DECLS
#endif /* __GST_SPLITMUXSINK_H__ */
