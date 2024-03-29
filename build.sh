#!/bin/bash
#
# Copyright (c) 2012 Leon Pajk. All rights reserved.
# Use of this source code is governed by a GPL-style license that can be
# found in the LICENSE file.


readonly SCRIPT_DIR="$(dirname "$0")"
readonly SCRIPT_DIR_ABS="$(cd "${SCRIPT_DIR}" ; pwd -P)"

export NACL_SDK_ROOT=/nacl_sdk
# NACL_TARGET_PLATFORM is really the name of a folder with the base dir -
# usually NACL_SDK_ROOT - within which the toolchain for the target platform
# are found.
# Replace the platform with the name of your target platform.  For example, to
# build applications that target the pepper_16 API, set
#   NACL_TARGET_PLATFORM="pepper_16"
export NACL_TARGET_PLATFORM="pepper_16"

readonly NACL_PLATFORM_DIR="${NACL_SDK_ROOT}/${NACL_TARGET_PLATFORM}"
readonly BASE_SCRIPT="${NACL_PLATFORM_DIR}/third_party/scons-2.0.1/script/scons"

export SCONS_LIB_DIR="${NACL_PLATFORM_DIR}/third_party/scons-2.0.1/engine"
export PYTHONPATH="${NACL_PLATFORM_DIR}/third_party/scons-2.0.1/engine"
# We have to do this because scons overrides PYTHONPATH and does not preserve
# what is provided by the OS.  The custom variable name won't be overwritten.
export PYMOX="${NACL_PLATFORM_DIR}/third_party/pymox"

"${BASE_SCRIPT}" --file=build.scons \
                 --site-dir="${NACL_PLATFORM_DIR}/build_tools/nacl_sdk_scons" \
                 $* | tee file

egrep -o "lib[^./]*\.?[^./]?\.[ao]" < file | sort -u

rm file
