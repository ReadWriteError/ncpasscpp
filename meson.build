project('ncpasscpp', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'],
  license : 'GNU GPLv3')

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_NCPASSCPP']

#add_global_arguments('-DSOME_TOKEN=value', language : 'cpp')

curlpp_dep = dependency('curlpp', version: '>= 0.8.1')
boost_dep = dependency('boost', version: '>= 1.74.0')

inc = include_directories('include')

fs = import('fs')

subdir('include')
subdir('src')
if fs.is_file('./.enable-tests')
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
