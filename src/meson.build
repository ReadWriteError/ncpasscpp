ncpasscpp_sources = ['API_Implementor.cpp', 'Session.cpp', 'Password.cpp']

ncpasscpp = shared_library(
  'ncpasscpp',
  ncpasscpp_sources,
  include_directories : inc,
  cpp_args : lib_args,
  dependencies : [curl_dep, nlohmann_json_dep, libsodium_dep, libssl_dep, thread_dep],
  install : true
)
