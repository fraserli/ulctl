project(
  'ulctl',
  'c',
  version: '0.1.0',
  default_options: ['warning_level=3']
)

cc = meson.get_compiler('c')

add_project_arguments(
  [
    '-DPROJECT_VERSION="@0@"'.format(meson.project_version()),
  ],
  language: 'c'
)

src = [
  'src/main.c',
  'src/error.c',
  'src/light.c',
]

deps = [
  dependency('libudev'),
  cc.find_library('m', required: false),
]

exe = executable(
  'ulctl',
  src,
  dependencies: deps,
  install: true
)

install_data('90-ulctl.rules', install_dir: '/usr/lib/udev/rules.d')
