################################################################################
# Autogenerated by build_tools/bazel_to_cmake/bazel_to_cmake.py from           #
# compiler/src/iree/compiler/Codegen/VMVX/BUILD                                #
#                                                                              #
# Use iree_cmake_extra_content from iree/build_defs.oss.bzl to add arbitrary   #
# CMake-only content.                                                          #
#                                                                              #
# To disable autogeneration for this file entirely, delete this header.        #
################################################################################

iree_add_all_subdirs()

iree_cc_library(
  NAME
    VMVX
  SRCS
    "LowerLinalgMicrokernels.cpp"
    "Passes.cpp"
    "VMVXAssignConstantOrdinals.cpp"
    "VMVXLinkExecutables.cpp"
  DEPS
    LLVMSupport
    MLIRArithDialect
    MLIRBufferizationDialect
    MLIRIR
    MLIRLinalgDialect
    MLIRMathDialect
    MLIRMemRefDialect
    MLIRPass
    MLIRTransforms
    iree::builtins::ukernel
    iree::compiler::Codegen::PassHeaders
    iree::compiler::Codegen::Utils
    iree::compiler::Dialect::Util::IR
    iree::compiler::Dialect::VM::IR
    iree::compiler::Dialect::VMVX::IR
    iree::compiler::Dialect::VMVX::IR::VMVXDialect
    iree::compiler::Utils
  PUBLIC
)

### BAZEL_TO_CMAKE_PRESERVES_ALL_CONTENT_BELOW_THIS_LINE ###
