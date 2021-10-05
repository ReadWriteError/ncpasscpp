dbus_cpp_dep = dependency('dbus-c++-1', version: '>= 0.9.0')

session_test1 = executable(
  'session_test1', 'ncpasscpp_session_test1.cpp',
  include_directories : inc,
  dependencies : [dbus_cpp_dep, curlpp_dep],
  link_with : ncpasscpp
)

password_test1 = executable(
  'password_test1', 'ncpasscpp_password_test1.cpp',
  include_directories : inc,
  dependencies : [dbus_cpp_dep, curlpp_dep],
  link_with : ncpasscpp
)

test('session test1', session_test1)
test('password test1', password_test1)
