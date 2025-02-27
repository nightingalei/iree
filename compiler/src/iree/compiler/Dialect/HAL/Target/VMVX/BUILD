# Copyright 2021 The IREE Authors
#
# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

load("//build_tools/bazel:build_defs.oss.bzl", "iree_cmake_extra_content", "iree_compiler_cc_library")

package(
    default_visibility = ["//visibility:public"],
    features = ["layering_check"],
    licenses = ["notice"],  # Apache 2.0
)

iree_cmake_extra_content(
    content = """
if(NOT IREE_TARGET_BACKEND_VMVX)
  return()
endif()
""",
)

iree_compiler_cc_library(
    name = "VMVX",
    srcs = [
        "VMVXTarget.cpp",
    ],
    hdrs = [
        "VMVXTarget.h",
    ],
    deps = [
        "//compiler/src/iree/compiler/Codegen:PassHeaders",
        "//compiler/src/iree/compiler/Codegen/Dialect:IREECodegenDialect",
        "//compiler/src/iree/compiler/Dialect/Flow/IR",
        "//compiler/src/iree/compiler/Dialect/HAL/Target",
        "//compiler/src/iree/compiler/Dialect/VM/Conversion",
        "//compiler/src/iree/compiler/Dialect/VM/IR",
        "//compiler/src/iree/compiler/Dialect/VM/Target/Bytecode",
        "//compiler/src/iree/compiler/Dialect/VM/Transforms",
        "//compiler/src/iree/compiler/Dialect/VMVX/IR:VMVXDialect",
        "//compiler/src/iree/compiler/Dialect/VMVX/Transforms",
        "//compiler/src/iree/compiler/Utils",
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:Pass",
        "@llvm-project//mlir:Support",
    ],
)
