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
  gboolean keyframe;         //if keyframe
  GstClockTimeDiff run_ts;   //_MqStreamCtx.in_running_time
  guint64 buf_size;          // buffer size
  GstClockTime duration;     // buffer duration
} MqStreamBuf;

typedef struct _MqStreamCtx
{
  gint refcount;

  GstSplitMuxSink *splitmux;

  guint q_overrun_id;     //_GstSplitMuxSink.queue.overrun_id
  guint sink_pad_block_id; //_GstSplitMuxSink.queue.sinkpad.probe_id.(DATA_DOWNSTREAM+EVENT_FLUSH+QUERY_DOWNSTREAM)
  guint src_pad_block_id; //_GstSplitMuxSink.queue.srcpad.probe_id.(DATA_DOWNSTREAM+EVENT_FLUSH)

  gboolean is_reference;  //_GstSplitMuxSink.reference_ctx. if the MqStreamCtx is reference context

  gboolean flushing;     //_GstSplitMuxSink.queue.srcpad probes EVENT_FLUSH, _GstSplitMuxSink.queue is flushing
  gboolean in_eos;       //_GstSplitMuxSink.queue.sinkpad probes EOS
  gboolean out_eos;
  gboolean need_unblock;
  gboolean caps_change;

  GstSegment in_segment;  //Segment _GstSplitMuxSink.queue.sinkpad probes and lets through
  GstSegment out_segment; //Segment _GstSplitMuxSink.queue.srcpad probes and lets through

  GstClockTimeDiff in_running_time;   //real-time valid running-time of GstBuffer streaming into _GstSplitMuxSink.queue
  GstClockTimeDiff out_running_time;  //real-time valid running-time of GstBuffer streaming outof _GstSplitMuxSink.queue and into _GstSplitMuxSink.muxer

  GstBuffer *prev_in_keyframe; /* store keyframe for each GOP */

  GstElement *q;    //_GstSplitMuxSink.queue
  GQueue queued_bufs;  //_MqStreamBuf, record GstBuffer info _GstSplitMuxSink.queue.sinkpad probes and lets through

  GstPad *sinkpad;  //_GstSplitMuxSink.queue.sinkpad
  GstPad *srcpad;   //_GstSplitMuxSink.queue.srcpad

  GstBuffer *cur_out_buffer;  //pointer to current buffer streaming outof _GstSplitMuxSink.queue and into _GstSplitMuxSink.muxer
  GstEvent *pending_gap;  
} MqStreamCtx;

//[1] properties
//GOP is the basic unit streaming into muxer
struct _GstSplitMuxSink
{
  GstBin parent;
  //[begin] for multiple streams synchronization
  GMutex lock;            //singleton lock for all pads input
  GCond input_cond;       //conditional variable for all pads input
  GCond output_cond;
  //[end] for multiple streams synchronization

  gdouble mux_overhead;    //[1] to calculate queue_bytes

  GstClockTime threshold_time;    //[1]
  guint64 threshold_bytes;    //[1]
  guint max_files;    //[1]
  gboolean send_keyframe_requests;    //[1]
  gchar *threshold_timecode_str;    //[1]
  GstClockTime next_max_tc_time;    //fragment_start_time + time_diff_calculated_according_to_[threshold_timecode_str]
  GstClockTime alignment_threshold;    //[1]

  GstElement *muxer;    //[2] pointer to muxer added to splitmuxsink, not increasing reference-count
  GstElement *sink;     //[2] pointer to active_sink or internal-sink element of active_sink

  GstElement *provided_muxer;   //[1]

  GstElement *provided_sink;    //[1]
  GstElement *active_sink;      //[2] pointer to sink bin

  gboolean ready_for_output;

  gchar *location;    //[1]
  guint fragment_id;  //fragment sequence number

  GList *contexts;    //q.MqStreamCtx

  SplitMuxInputState input_state;        //q.sinkpad state variable
  GstClockTimeDiff max_in_running_time;  //realtime max running-time of GstBuffer of reference stream streaming into queues
                                         //it's updated continuously and kept unchanged when sinkpad receiving keyframe in loop 
                                         //to wait for non-reference stream catching up
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
  GstClockTimeDiff max_out_running_time;  //current max running time for output gop

  guint64 muxed_out_bytes;     //buffer size streaming into muxer

  MqStreamCtx *reference_ctx;  //video pad MqStreamCtx associated with internal queue or first non-video pad MqStreamCtx when no video stream
  /* Count of queued keyframes in the reference ctx */
  guint queued_keyframes;

  gboolean switching_fragment;  //indicates switching fragment now

  gboolean have_video;    //video pad requested only once

  gboolean need_async_start;
  gboolean async_pending;

  gboolean use_robust_muxing;    //[1]
  gboolean muxer_has_reserved_props; //muxer has properties "reserved-max-duration" and "reserved-duration-remaining"

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
