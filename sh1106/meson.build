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
    '-D_LINUX_',
]

app_deps = [
    dependency('tinyc'),
]

app_sources = [
    'ss_oled.c',
    'main.c',
]

executable(
    meson.project_name(),
    c_args: c_args,
    dependencies: app_deps,
    sources: app_sources,
    install: false,
)

