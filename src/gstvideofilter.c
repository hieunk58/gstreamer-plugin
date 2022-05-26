#include "gstvideofilter.h"

static void gst_video_filter_class_init (GstVideoFilterClass *kclass);

static void
gst_video_filter_class_init (GstVideoFilterClass *kclass)
{
    GST_DEBUG ("gst_video_filter_class_init");

    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

    gst_element_class_set_static_metadata (element_class,
        "A video plugin",
        "Generic",
        "A plugin for video streaming",
        "Hieu Nguyen <khachieunk@gmail.com>");
}
