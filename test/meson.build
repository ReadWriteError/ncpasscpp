dbus_cpp_dep = dependency('dbus-c++-1', version: '>= 0.9.0')

testexe = executable(
  'testexe', 'ncpasscpp_test.cpp',
  include_directories : inc,
  dependencies : [dbus_cpp_dep, curlpp_dep],
  link_with : ncpasscpp
)

test('ncpasscpp test', testexe)
