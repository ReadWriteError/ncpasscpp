project('ncpasscpp', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c17', 'cpp_std=c++17'],
  license : 'GNU GPLv3')

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_NCPASSCPP']

#add_global_arguments('-DSOME_TOKEN=value', language : 'cpp')

curlpp_dep = dependency('curlpp', version: '>= 0.8.1')
nlohmann_json_dep = dependency('nlohmann_json', version: '>= 3.9.1')
libsodium_dep = dependency('libsodium', version: '>= 1.0.18')
libssl_dep = dependency('openssl')
thread_dep = dependency('threads')

inc = include_directories('include')

fs = import('fs')

subdir('include')
subdir('src')
if fs.is_file('./test/user-specific.hpp')
  subdir('test')
endif

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : 'libncpasscpp-dev',
  filebase : 'ncpasscpp',
  description : 'Nextcloud Passwords API library.',
  libraries : ncpasscpp,
  version : '0.1',
)
