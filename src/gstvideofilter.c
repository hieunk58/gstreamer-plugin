#include "gstvideofilter.h"

#define BLOCKSIZE 2048

/* TODO example properties definition */
enum
{
    PROP_0,
    PROP_1,
    PROP_2
};

static void gst_video_filter_class_init (GstVideoFilterClass *kclass);
static void gst_video_filter_init (GstVideoFilter *filter);
static void gst_video_filter_set_property (GObject *object, guint prop_id,
    const GValue *value,
    GParamSpec *pspec);
static void gst_video_filter_get_property (GObject *object, guint prop_id,
    GValue *value,
    GParamSpec *pspec);
static GstFlowReturn gst_video_filter_sink_chain (GstPad *pad, 
    GstObject *parent, GstBuffer *buf);
static gboolean gst_video_filter_sink_event (GstPad *pad,
    GstObject *parent, GstEvent *event);
static gboolean gst_video_filter_src_event (GstPad *pad,
    GstObject *parent, GstEvent *event);
static gboolean gst_video_filter_query (GstPad *pad,
    GstObject *parent, GstQuery *query);
static gboolean gst_video_filter_activate (GstPad *pad, GstObject *parent);
static gboolean gst_video_filter_activate_mode (GstPad *pad, 
    GstObject *parent, GstPadMode mode, gboolean active);
static void gst_video_filter_loop (GstVideoFilter *filter);
static GstFlowReturn gst_video_filter_get_range (GstPad *pad, GstObject *parent,
    guint64 offset, guint length, GstBuffer **buf);
static gboolean gst_video_filter_query_caps (GstPad *pad, GstObject *parent,
    GstQuery *query);

/* Pad templates */
static GstStaticPadTemplate src_factory =
GST_STATIC_PAD_TEMPLATE (
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (
        "video/x-raw, "
        "format = (string) I420, "
        "framerate = (fraction) [ 0/1, 2147483647/1 ], " 
        "width = (int) [ 1, 2147483647 ], " 
        "height = (int) [ 1, 2147483647 ]"
    )
);

static GstStaticPadTemplate sink_factory =
GST_STATIC_PAD_TEMPLATE (
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (
        "video/x-raw, "
        "format = (string) I420, "
        "framerate = (fraction) [ 0/1, 2147483647/1 ], " 
        "width = (int) [ 1, 2147483647 ], " 
        "height = (int) [ 1, 2147483647 ]"
    )
);

static void
gst_video_filter_class_init (GstVideoFilterClass *kclass)
{
    GST_DEBUG ("gst_video_filter_class_init");

    GstElementClass *element_class = GST_ELEMENT_CLASS (kclass);
    GObjectClass *object_class = G_OBJECT_CLASS (kclass);

    gst_element_class_add_pad_template (element_class,
    gst_static_pad_template_get (&src_factory));

    gst_element_class_add_pad_template (element_class,
    gst_static_pad_template_get (&sink_factory));

    gst_element_class_set_static_metadata (element_class,
        "A video plugin",
        "Generic",
        "A plugin for video streaming",
        "Hieu Nguyen <khachieunk@gmail.com>");
    
    /* define virtual function pointers */
    object_class->set_property = gst_video_filter_set_property;
    object_class->get_property = gst_video_filter_get_property;

    /* define properties */
    g_object_class_install_property (object_class, PROP_0,
        g_param_spec_boolean ("silent", "Silent",
        "example boolean property",
        FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (object_class, PROP_1,
        g_param_spec_float("extend-border", "Extend Border Factor", "How much the border "
        "of the video should be extended.",
        0.10f, 0.99f, 0.25f, G_PARAM_READWRITE));

    g_object_class_install_property (object_class, PROP_2,
        g_param_spec_int("frame-window-size", "Frame Window Size", "How many frames to retarget "
        "at a time from an incoming stream.",
        10, 500, 100, G_PARAM_READWRITE));
}

static void gst_video_filter_init (GstVideoFilter *filter) {
    GST_DEBUG ("gst_video_filter_init");

    /* Sinkpad */
    filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");

    gst_pad_use_fixed_caps(filter->sinkpad);

    gst_pad_set_event_function (filter->sinkpad, GST_DEBUG_FUNCPTR (gst_video_filter_sink_event));
    gst_pad_set_chain_function (filter->sinkpad, GST_DEBUG_FUNCPTR (gst_video_filter_sink_chain));
    gst_pad_set_query_function (filter->sinkpad, GST_DEBUG_FUNCPTR (gst_video_filter_query));
    gst_pad_set_activate_function (filter->sinkpad, GST_DEBUG_FUNCPTR (gst_video_filter_activate));
    gst_pad_set_activatemode_function (filter->sinkpad, GST_DEBUG_FUNCPTR (gst_video_filter_activate_mode));

    gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

    /* TODO Configure Sinkpad */
    
    /* Sourcepad */
    filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");

    gst_pad_use_fixed_caps(filter->srcpad);

    gst_pad_set_event_function (filter->srcpad, GST_DEBUG_FUNCPTR (gst_video_filter_src_event));
    gst_pad_set_query_function (filter->srcpad, GST_DEBUG_FUNCPTR(gst_video_filter_query));
    gst_pad_set_activate_function (filter->srcpad, GST_DEBUG_FUNCPTR (gst_video_filter_activate));
    gst_pad_set_activatemode_function (filter->srcpad, GST_DEBUG_FUNCPTR (gst_video_filter_activate_mode));
    gst_pad_set_getrange_function (filter->srcpad, GST_DEBUG_FUNCPTR (gst_video_filter_get_range));
    gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

    /* TODO Configure Sourcepad */

    filter->silent = FALSE;
}

static void gst_video_filter_set_property (GObject *object, guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
    GST_DEBUG ("gst_video_filter_set_property");
    GstVideoFilter *filter = GST_VIDEO_FILTER (object);

    switch (prop_id)
    {
    case PROP_0:
        filter->silent = g_value_get_boolean (value);
        g_print ("Silent argument was changed to %s\n",
            filter->silent ? "true" : "false");
        break;
    case PROP_1:
        break;
    case PROP_2:
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void gst_video_filter_get_property (GObject *object, guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
    GST_DEBUG ("gst_video_filter_get_property");
    GstVideoFilter *filter = GST_VIDEO_FILTER (object);

    switch (prop_id)
    {
    case PROP_0:
        g_value_set_boolean (value, filter->silent);
        break;
    
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }

}

static GstFlowReturn gst_video_filter_sink_chain (GstPad *pad, 
    GstObject *parent, GstBuffer *buf)
{
    GstVideoFilter *filter = GST_VIDEO_FILTER (parent);
    GstBuffer *outbuf = NULL;

    // process data with buf then assign to outbuf

    gst_buffer_unref (buf);

    if (!outbuf) {
        /* something went wrong - signal an error */
        GST_ELEMENT_ERROR (GST_ELEMENT (filter), STREAM, FAILED, (NULL), (NULL));
        return GST_FLOW_ERROR;
    }
    
    return gst_pad_push (filter->srcpad, outbuf);
}

static gboolean gst_video_filter_sink_event (GstPad *pad,
    GstObject *parent, GstEvent *event)
{
    gboolean ret = TRUE;
    GstVideoFilter *filter = GST_VIDEO_FILTER (parent);

    switch (GST_EVENT_TYPE (event))
    {
    case GST_EVENT_CAPS:
        /* handle the format here */
        ret = gst_pad_push_event (filter->srcpad, event);
        break;
    case GST_EVENT_EOS:
        /* end of stream, close down all stream */
        // TODO call function gst_video_filter_stop_processing (filter);
        ret = gst_pad_push_event (filter->srcpad, event);
        break;
    default:
        ret = gst_pad_event_default (pad, parent, event);
        break;
    }

    return ret;
}

static gboolean gst_video_filter_src_event (GstPad *pad,
    GstObject *parent, GstEvent *event)
{
    gboolean ret = TRUE;
    GstVideoFilter *filter = GST_VIDEO_FILTER (parent);

    // TODO
    switch (GST_EVENT_TYPE (event))
    {
    case GST_EVENT_SEEK:
        break;
    case GST_EVENT_NAVIGATION:       
        break;
    default:
        ret = gst_pad_event_default (pad, parent, event);
        break;
    }

    ret = gst_pad_push_event (filter->sinkpad, event);

    return ret;
}

static gboolean gst_video_filter_query (GstPad *pad,
    GstObject *parent, GstQuery *query)
{
    gboolean ret = TRUE;
    GstVideoFilter *filter = GST_VIDEO_FILTER (parent);

    switch (GST_QUERY_TYPE (query))
    {
    case GST_QUERY_POSITION:
        /* report the current position */
        break;
    case GST_QUERY_DURATION:
        /* report the current duration */
        break;
    case GST_QUERY_CAPS:
    {
        /* report the supported caps */
        GstPad *otherpad;
        GstCaps *temp, *caps, *filt, *tcaps;
        gint i;

        otherpad = (pad == filter->srcpad) ? filter->sinkpad : filter->srcpad;
        caps = gst_pad_get_allowed_caps (otherpad);

        gst_query_parse_caps (query, &filt);
        for (i = 0; i < gst_caps_get_size (caps); ++i) {
            GstStructure *structure = gst_caps_get_structure (caps, i);
            gst_structure_remove_field (structure, "rate");
        }

        /* return intersect results with padtemplate */
        tcaps = gst_pad_get_pad_template_caps (pad);
        if (tcaps) {
            temp = gst_caps_intersect (caps, tcaps);
            gst_caps_unref (caps);
            gst_caps_unref (tcaps);
            caps = temp;
        }
        if (filt) {
            temp = gst_caps_intersect (caps, filt);
            gst_caps_unref (caps);
            caps = temp;
        }
        gst_query_set_caps_result (query, caps);
        gst_caps_unref (caps);
        ret = TRUE;
        break;
    }
    default:
        ret = gst_pad_query_default (pad, parent, query);
        break;
    }

    return ret;
}

static gboolean gst_video_filter_activate (GstPad *pad, GstObject *parent)
{
    GstQuery *query;
    gboolean pull_mode;

    /* check what upstream scheduling is supported */
    query = gst_query_new_scheduling ();

    if (!gst_pad_peer_query (pad, query)) {
        gst_query_unref (query);
        goto activate_push;
    }

    /* check if pull mode is supported */
    pull_mode = gst_query_has_scheduling_mode_with_flags (query, GST_PAD_MODE_PULL, 
        GST_SCHEDULING_FLAG_SEEKABLE);
    gst_query_unref (query);

    if (!pull_mode) goto activate_push;

    /* activate in pull mode now. GStreamer will also activate the upstream peer in pull mode */
    return gst_pad_activate_mode (pad, GST_PAD_MODE_PULL, TRUE);

activate_push:
    {
        /* something went wrong, fallback to push mode */
        return gst_pad_activate_mode (pad, GST_PAD_MODE_PUSH, TRUE);
    }
}

static gboolean gst_video_filter_activate_mode (GstPad *pad, 
    GstObject *parent, GstPadMode mode, gboolean active)
{
    gboolean ret = FALSE;
    GstVideoFilter *filter = GST_VIDEO_FILTER (parent);

    switch (mode)
    {
    case GST_PAD_MODE_PULL:
        if (active) {
            filter->offset = 0;
            ret = gst_pad_start_task (pad, 
                    (GstTaskFunction) gst_video_filter_loop, filter, NULL);
        } else {
            ret = gst_pad_stop_task (pad);
        }     
        break;
    case GST_PAD_MODE_PUSH:
        ret = TRUE;
        break;
    default:
        /* unknown scheduling mode */
        break;
    }

    return ret;
}

static void gst_video_filter_loop (GstVideoFilter *filter)
{
    GstFlowReturn ret = TRUE;
    guint64 len;
    GstFormat format = GST_FORMAT_BYTES;
    GstBuffer *buf = NULL;

    if (!gst_pad_query_duration (filter->sinkpad, format, &len)) {
        GST_DEBUG_OBJECT (filter, "failed to query duration, pausing"); 
        goto stop;
    }

    if (filter->offset >= len) {
        GST_DEBUG_OBJECT (filter, "at end of input, sending EOS, pausing");
        gst_pad_push_event (filter->srcpad, gst_event_new_eos ());
        goto stop;
    }

    /* read BLOCKSIZE bytes from byte offset */
    ret = gst_pad_pull_range (filter->sinkpad, filter->offset, BLOCKSIZE, &buf);
    if (ret != GST_FLOW_OK) {
        GST_DEBUG_OBJECT (filter, "pull_range failed: %s", gst_flow_get_name (ret));
        goto stop;
    }
    /* get pull range ok, now push buffer downstream */
    ret = gst_pad_push (filter->srcpad, buf);
    buf = NULL;

    if (ret != GST_FLOW_OK) {
        GST_DEBUG_OBJECT (filter, "pad_push failed: %s", gst_flow_get_name (ret));
        goto stop;
    }

    /* everything is fine, increase offset and wait to be called again */
    filter->offset += BLOCKSIZE;
    return;

stop:
    GST_DEBUG_OBJECT (filter, "pausing task");
    gst_pad_pause_task (filter->sinkpad);
}

static GstFlowReturn gst_video_filter_get_range (GstPad *pad, GstObject *parent,
    guint64 offset, guint length, GstBuffer **buf)
{
    /* TODO */
    return GST_FLOW_OK;
}

static gboolean plugin_init (GstPlugin *plugin)
{
    return gst_element_register (plugin, "video_filter", GST_RANK_NONE, GST_TYPE_VIDEO_FILTER);
}


#ifndef PACKAGE
#define PACKAGE "video_filter"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    video_filter,
    "A plugin for video streaming",
    plugin_init,
    "1.0",
    "LGPL",
    "gst-video-plugin",
    "https://github.com/hieunk58/gstreamer-plugin"
)
