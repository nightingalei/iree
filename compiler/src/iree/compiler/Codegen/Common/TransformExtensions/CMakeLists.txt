################################################################################
# Autogenerated by build_tools/bazel_to_cmake/bazel_to_cmake.py from           #
# compiler/src/iree/compiler/Codegen/Common/TransformExtensions/BUILD          #
#                                                                              #
# Use iree_cmake_extra_content from iree/build_defs.oss.bzl to add arbitrary   #
# CMake-only content.                                                          #
#                                                                              #
# To disable autogeneration for this file entirely, delete this header.        #
################################################################################

iree_add_all_subdirs()

iree_tablegen_library(
  NAME
    CommonExtensionsOpGen
  TD_FILE
    "CommonExtensionsOps.td"
  OUTS
    --gen-op-decls CommonExtensionsOps.h.inc
    --gen-op-defs CommonExtensionsOps.cpp.inc
)

iree_cc_library(
  NAME
    CommonExtensions
  HDRS
    "CommonExtensions.h"
    "CommonExtensionsOps.h.inc"
  SRCS
    "CommonExtensions.cpp"
    "CommonExtensionsOps.cpp.inc"
  DEPS
    ::CommonExtensionsOpGen
    IREEDialectsTransforms
    IREELinalgTransformDialect
    LLVMSupport
    MLIRAffineDialect
    MLIRArithDialect
    MLIRArithUtils
    MLIRBufferizationDialect
    MLIRBufferizationTransforms
    MLIRLinalgTransformOps
    MLIRLinalgTransforms
    MLIRMemRefDialect
    MLIRMemRefTransforms
    MLIRPass
    MLIRTensorDialect
    MLIRTransformDialect
    iree::compiler::Codegen::Common::CommonPasses
    iree::compiler::Codegen::Interfaces::BufferizationInterfaces
    iree::compiler::Codegen::PassHeaders
    iree::compiler::Codegen::Utils
    iree::compiler::Dialect::Flow::IR
    iree::compiler::Dialect::HAL::IR
  PUBLIC
)

### BAZEL_TO_CMAKE_PRESERVES_ALL_CONTENT_BELOW_THIS_LINE ###
