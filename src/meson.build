ncpasscpp_sources = ['API_Implementor.cpp', 'Session.cpp']

ncpasscpp = shared_library(
  'ncpasscpp',
  ncpasscpp_sources,
  include_directories : inc,
  cpp_args : lib_args,
  dependencies : [curlpp_dep],
  install : true
)
