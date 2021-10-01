dbus_cpp_dep = dependency('dbus-c++-1', version: '>= 0.9.0')

session_test1 = executable(
  'session_test1', 'ncpasscpp_session_test1.cpp',
  include_directories : inc,
  dependencies : [dbus_cpp_dep, curlpp_dep],
  link_with : ncpasscpp
)

test('ncpasscpp test', session_test1)
