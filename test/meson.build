dbus_cpp_dep = dependency('dbus-c++-1', version: '>= 0.9.0')

session_test1 = executable(
  'test_session_1', 'test_session_1.cpp',
  include_directories : inc,
  dependencies : [dbus_cpp_dep],
  link_with : ncpasscpp
)

password_test1 = executable(
  'test_password_1', 'test_password_1.cpp',
  include_directories : inc,
  dependencies : [dbus_cpp_dep],
  link_with : ncpasscpp
)

password_test2 = executable(
  'test_password_2', 'test_password_2.cpp',
  include_directories : inc,
  dependencies : [dbus_cpp_dep],
  link_with : ncpasscpp
)

test('session-creation', session_test1, suite: 'read')
test('password-read', password_test1, suite: 'read')
test('password-create', password_test2, suite: 'write')
