project(
    'tinychip',
    ['c'],
    version : '1.0',
    license : 'LGPL-2.0',
    default_options : [
        'c_std=c99',
    ],
)

c_args = [
    '-Wall',
    '-Wextra',
    '-D_GNU_SOURCE',
    '-D__STDC_FORMAT_MACROS',
]

if get_option('buildtype') == 'plain'
  c_args += '-O2'
endif

tinychip_deps = [
    #dependency('libpcre'),
]

tinychip_headers = [
    'lib/libgpio.h',
    'lib/libi2c.h',
    'lib/msleep.h',
]

tinychip_sources = [
]

# Build and install library ---------------------------------------------------

tinychip = static_library(
    'tinychip',
    include_directories : ['lib'],
    c_args : c_args,
    dependencies : tinychip_deps,
    sources : tinychip_sources,
    install : true,
)

install_headers(tinychip_headers, subdir: 'tinychip')

pkg_mod = import('pkgconfig')
pkg_mod.generate(
    libraries : tinychip,
    version : '1.0',
    name : 'libtinychip',
    filebase : 'tinychip',
    subdirs : 'tinychip',
    description : 'A simple C library.'
)

# Build sample program --------------------------------------------------------

testcmd_sources = [
    'main.c',
]

executable(
    'testcmd',
    include_directories : ['lib'],
    c_args : c_args,
    sources : testcmd_sources,
    link_with : [tinychip],
)


