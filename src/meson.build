plugin_c_args = ['-DHAVE_CONFIG_H']

plugin_sources = [
  'gstvideofilter.c'
  ]

gstvideoplugin = library('gstplugin',
  plugin_sources,
  c_args: plugin_c_args,
  dependencies : [gst_dep],
  install : true,
  install_dir : plugins_install_dir,
)
