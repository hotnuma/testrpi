project(
    'testcmd',
    ['c'],
    default_options: ['c_std=c99'],
    version: '1.0',
    license: 'GPL-2.0',
)

c_args = [
    '-Wall',
    '-Wextra',
    '-O2',
    '-D_GNU_SOURCE',
]

app_includes = [
    '../i2c',
]

app_sources = [
    'mcp9808.c',
    'main.c',
]

executable(
    meson.project_name(),
    c_args: c_args,
    include_directories : app_includes,
    sources: app_sources,
    install: false,
)

