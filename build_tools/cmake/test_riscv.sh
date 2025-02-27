#!/bin/bash

# Copyright 2021 The IREE Authors
#
# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# Test the cross-compiled RISCV 64-bit Linux targets.

set -xeuo pipefail

# Print the UTC time when set -x is on
export PS4='[$(date -u "+%T %Z")] '

ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel)}"
BUILD_RISCV_DIR="${BUILD_RISCV_DIR:-$ROOT_DIR/build-riscv}"
RISCV_ARCH="${RISCV_ARCH:-rv64}"

# Environment variable used by the emulator.
export RISCV_TOOLCHAIN_ROOT="${RISCV_RV64_LINUX_TOOLCHAIN_ROOT}"

export CTEST_PARALLEL_LEVEL=${CTEST_PARALLEL_LEVEL:-$(nproc)}

ctest_args=(
  "--timeout 900"
  "--output-on-failure"
  "--no-tests=error"
)

declare -a label_exclude_args=(
  "^nokokoro$"
  "^driver=vulkan$"
  "^driver=cuda$"
  "^vulkan_uses_vk_khr_shader_float16_int8$"
  "^requires-filesystem$"
  "^requires-dtz$"
)

# Excluding mobilebert, fp16, and lowering_config regression
# tests for now.
# TODO(#10462): Investigate the lowering_config test issue.
declare -a test_exclude_args=(
  "bert"
  "fp16"
  "regression_llvm-cpu_lowering_config"
)

# Test runtime unit tests
runtime_label_exclude_regex="($(IFS="|" ; echo "${label_exclude_args[*]}"))"
runtime_ctest_args=(
  "--test-dir ${BUILD_RISCV_DIR}/runtime/"
  ${ctest_args[@]}
  "--label-exclude ${runtime_label_exclude_regex}"
)
echo "******** Running runtime CTest ********"
ctest ${runtime_ctest_args[@]}

tools_ctest_args=(
  "--test-dir ${BUILD_RISCV_DIR}/tools/test"
  ${ctest_args[@]}
  "--label-exclude ${runtime_label_exclude_regex}"
)
echo "******** Running tools CTest ********"
ctest ${tools_ctest_args[@]}

if [[ "${RISCV_ARCH}" == "rv32-linux" ]]; then
  # mhlo.power is also disabled because musl math library is not compiled for
  # 32-bit.
  test_exclude_args+=(
    "xla.*llvm-cpu.*pow"
  )
fi

tests_label_exclude_regex="($(IFS="|" ; echo "${label_exclude_args[*]}"))"
tests_exclude_regex="($(IFS="|" ; echo "${test_exclude_args[*]}"))"
test_ctest_args=(
  "--test-dir ${BUILD_RISCV_DIR}/tests/e2e"
  ${ctest_args[@]}
  "--label-exclude ${tests_label_exclude_regex}"
  "--exclude-regex ${tests_exclude_regex}"
)
echo "******** Running e2e CTest ********"
ctest  ${test_ctest_args[@]}
