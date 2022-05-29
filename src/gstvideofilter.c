#include "gstvideofilter.h"

static void gst_video_filter_class_init (GstVideoFilterClass *kclass);
static void gst_video_filter_init (GstVideoFilter *filter);
static GstFlowReturn gst_video_filter_sink_chain (GstPad *pad, 
    GstObject *parent, GstBuffer *buf);
static gboolean gst_video_filter_sink_event (GstPad *pad,
    GstObject *parent, GstEvent *event);
static gboolean gst_video_filter_src_event (GstPad *pad,
    GstObject *parent, GstEvent *event);
static gboolean gst_video_filter_query (GstPad *pad,
    GstObject *parent, GstQuery *query);

/* Pad templates */
static GstStaticPadTemplate src_factory =
GST_STATIC_PAD_TEMPLATE (
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY") // TODO specify supported video type
);

static GstStaticPadTemplate sink_factory =
GST_STATIC_PAD_TEMPLATE (
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY") // TODO specify supported video type
);

static void
gst_video_filter_class_init (GstVideoFilterClass *kclass)
{
    GST_DEBUG ("gst_video_filter_class_init");

    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

    gst_element_class_add_pad_template (element_class,
    gst_static_pad_template_get (&src_factory));

    gst_element_class_add_pad_template (element_class,
    gst_static_pad_template_get (&sink_factory));

    gst_element_class_set_static_metadata (element_class,
        "A video plugin",
        "Generic",
        "A plugin for video streaming",
        "Hieu Nguyen <khachieunk@gmail.com>");
}

static void gst_video_filter_init (GstVideoFilter *filter) {
    GST_DEBUG ("gst_video_filter_init");

    /* Sinkpad */
    filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
    gst_pad_set_event_function (filter->sinkpad, GST_DEBUG_FUNCPTR (gst_video_filter_sink_event));
    gst_pad_set_chain_function (filter->sinkpad, GST_DEBUG_FUNCPTR (gst_video_filter_sink_chain));
    gst_pad_set_query_function (filter->sinkpad, GST_DEBUG_FUNCPTR (gst_video_filter_query));

    gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

    /* TODO Configure Sinkpad */
    
    /* Sourcepad */
    filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
    gst_pad_set_event_function (filter->srcpad, GST_DEBUG_FUNCPTR (gst_video_filter_src_event));
    gst_pad_set_query_function (filter->srcpad, GST_DEBUG_FUNCPTR(gst_video_filter_query));

    gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

    /* TODO Configure Sourcepad */

    filter->silent = FALSE;
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
        /* report the supported caps */
        break;
    default:
        ret = gst_pad_query_default (pad, parent, query);
        break;
    }

    return ret;
}

static boolean plugin_init (GstPlugin *plugin)
{
    return GST_ELEMENT_REGISTER (video_filter, plugin);
}

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
