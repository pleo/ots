#!/usr/bin/python -Wall
import os
from string import Template

## Base directory for the project
root_dir = os.getcwd() + os.sep

## Directories for the include header files
include_search_path =  [#'/usr/include/libxml2',
                        '#/include/']

## Dictionary directory for all supported languages
dictionary_base_dir = root_dir + 'dict' + os.sep

## Predefined preprocessor macros
defs = ['-DDICTIONARY_DIR=\\"' + dictionary_base_dir + '\\"']

## These are our source files
sources = str.split("""article.c
                       dictionary.c
                       grader.c
                       grader-tf.c
                       grader-tc.c
                       parser.c
                       text.c
                       stemmer.c
                       highlighter.c
                       wordlist.c
                       relations.c
                       ots.c""")

## These are our test files
#test_sources = ['Server.cpp','#tests/testrunner.cpp']

## These are our static libraries
static_libs = [] #'/usr/lib/libxml2.so']

#### If you want to start your own build setup with a
#### different layout than mine.
source_base_dir = 'src'
build_base_dir = 'build'
target_name = 'ots'
nacl = False

nacl_sdk_root = os.sep.join(os.getcwd().split(os.sep)[:4])
# NACL_TARGET_PLATFORM is really the name of a folder with the base dir -
# usually NACL_SDK_ROOT - within which the toolchain for the target platform
# are found.
# Replace the platform with the name of your target platform.  For example, to
# build applications that target the pepper_16 API, set
#   NACL_TARGET_PLATFORM="pepper_16"
nacl_target_platform = 'pepper_16'
nacl_platform_dir = nacl_sdk_root + os.sep + nacl_target_platform
runtime_library = Template(os.sep.join([nacl_platform_dir,
                                        'toolchain',
                                        '$name',
                                        '']))
nacl_toolchain_dir = runtime_library.substitute(name = 'linux_x86_newlib')
nacl_toolchain_lib_dir = nacl_toolchain_dir + \
                         os.sep.join(['x86_64-nacl', 'lib'])
nacl_toolchain_bin_dir = nacl_toolchain_dir + 'bin' + os.sep
nacl_toolchain_usr_dir = nacl_toolchain_dir + \
                         os.sep.join(['i686-nacl', 'usr', ''])
nacl_toolchain_usr_lib_dir = nacl_toolchain_usr_dir + 'lib' + os.sep
nacl_toolchain_usr_bin_dir = nacl_toolchain_usr_dir + os.sep.join(['bin', ''])
base_script = os.sep.join([nacl_platform_dir,
                          'third_party',
                          'scons-2.0.1',
                          'script',
                          'scons'])
pythonpath = base_script
# We have to do this because scons overrides PYTHONPATH and does not preserve
# what is provided by the OS.  The custom variable name won't be overwritten.
pymox = os.sep.join([nacl_platform_dir,
                    'third_party',
                    'pymox'])

