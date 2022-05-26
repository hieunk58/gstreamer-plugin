#include "gstvideofilter.h"

static void gst_video_filter_class_init (GstVideoFilterClass *kclass);
static void gst_video_filter_init (GstVideoFilter *filter);

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
    gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

    /* TODO Configure Sinkpad */
    
    /* Sourcepad */
    filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
    gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

    /* TODO Configure Sourcepad */

    filter->silent = FALSE;
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
