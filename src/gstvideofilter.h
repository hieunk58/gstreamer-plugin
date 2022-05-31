#ifndef _GST_VIDEO_FILTER_H_
#define _GST_VIDEO_FILTER_H_

#include <gst/gst.h>
#include <glib.h>

G_BEGIN_DECLS

typedef struct _GstVideoFilter {
    GstElement element;

    GstPad *sinkpad, *srcpad;

    guint offset;

    gboolean silent;

} GstVideoFilter;


typedef struct _GstVideoFilterClass {
    GstElementClass base_class;

} GstVideoFilterClass;

#define GST_TYPE_VIDEO_FILTER               (gst_video_filter_get_type())

#define GST_VIDEO_FILTER(obj)               (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_VIDEO_FILTER, GstVideoFilter))
#define GST_VIDEO_FILTER_CLASS(kclass)      (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_VIDEO_FILTER, GstVideoFilterClass))

#define GST_IS_VIDEO_FILTER(obj)            (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_VIDEO_FILTER))
#define GST_IS_VIDEO_FILTER_CLASS(obj)      (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_VIDEO_FILTER))

GType gst_video_filter_get_type (void);

G_END_DECLS

#endif // _GST_VIDEO_FILTER_H_
