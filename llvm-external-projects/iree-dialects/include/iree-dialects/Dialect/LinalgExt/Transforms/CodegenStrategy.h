// Copyright 2022 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef IREE_DIALECTS_DIALECT_LINALGEXT_TRANSFORMS_CODEGENSTRATEGY_H_
#define IREE_DIALECTS_DIALECT_LINALGEXT_TRANSFORMS_CODEGENSTRATEGY_H_

#include "iree-dialects/Dialect/LinalgExt/Passes/Passes.h"
#include "mlir/Pass/PassManager.h"

#include <utility>

//===----------------------------------------------------------------------===//
// Strategies moved from upstream MLIR as IREE still heavily relies on patterns
// that compose through filters.
// TODO: Deprecate everything below.
//===----------------------------------------------------------------------===//

namespace mlir {
namespace iree_compiler {
namespace IREE {
namespace LinalgExt {

/// Abstract Transformation class applied in a sequence that also handles state
/// through markers.
struct Transformation {
  explicit Transformation(linalg::LinalgTransformationFilter::FilterFunction f)
      : filter(std::move(f)) {}
  virtual ~Transformation() = default;
  virtual void
  addToPassPipeline(OpPassManager &pm,
                    linalg::LinalgTransformationFilter m) const = 0;
  linalg::LinalgTransformationFilter::FilterFunction filter = nullptr;
};

/// Represent one application of LinalgStrategyTileAndFusePass.
struct TileAndFuse : public Transformation {
  TileAndFuse(StringRef name, linalg::LinalgTilingAndFusionOptions options,
              linalg::LinalgTransformationFilter::FilterFunction f = nullptr)
      : Transformation(std::move(f)), opName(name),
        options(std::move(options)) {}

  void addToPassPipeline(OpPassManager &pm,
                         linalg::LinalgTransformationFilter m) const override {
    pm.addPass(createLinalgStrategyTileAndFusePass(opName, options, m));
  }

private:
  std::string opName;
  linalg::LinalgTilingAndFusionOptions options;
};

/// Represent one application of LinalgStrategyTilePass.
struct Tile : public Transformation {
  Tile(StringRef name, linalg::LinalgTilingOptions options,
       linalg::LinalgTransformationFilter::FilterFunction f = nullptr)
      : Transformation(std::move(f)), opName(name),
        options(std::move(options)) {}

  void addToPassPipeline(OpPassManager &pm,
                         linalg::LinalgTransformationFilter m) const override {
    pm.addPass(createLinalgStrategyTilePass(opName, options, m));
  }

private:
  std::string opName;
  linalg::LinalgTilingOptions options;
};

/// Represent one application of LinalgStrategyPadPass.
struct Pad : public Transformation {
  Pad(StringRef name, linalg::LinalgPaddingOptions options,
      linalg::LinalgTransformationFilter::FilterFunction f = nullptr)
      : Transformation(std::move(f)), opName(name),
        options(std::move(options)) {}

  void addToPassPipeline(OpPassManager &pm,
                         linalg::LinalgTransformationFilter m) const override {
    pm.addPass(createLinalgStrategyPadPass(opName, options, m));
  }

private:
  std::string opName;
  linalg::LinalgPaddingOptions options;
};

/// Represent one application of createLinalgStrategyDecomposePass.
struct Decompose : public Transformation {
  explicit Decompose(
      linalg::LinalgTransformationFilter::FilterFunction f = nullptr)
      : Transformation(std::move(f)) {}

  void addToPassPipeline(OpPassManager &pm,
                         linalg::LinalgTransformationFilter m) const override {
    pm.addPass(createLinalgStrategyDecomposePass(m));
  }
};

/// Represent one application of createLinalgStrategyPeelPass.
struct Peel : public Transformation {
  explicit Peel(LinalgPeelOptions options,
                linalg::LinalgTransformationFilter::FilterFunction f = nullptr)
      : Transformation(std::move(f)), options(options) {}

  Peel(StringRef name, LinalgPeelOptions options,
       linalg::LinalgTransformationFilter::FilterFunction f = nullptr)
      : Transformation(std::move(f)), opName(name), options(options) {}

  void addToPassPipeline(OpPassManager &pm,
                         linalg::LinalgTransformationFilter m) const override {
    pm.addPass(createLinalgStrategyPeelPass(opName, options, m));
  }

private:
  std::string opName;
  LinalgPeelOptions options;
};

/// Represent one application of createLinalgStrategyVectorizePass.
struct Vectorize : public Transformation {
  explicit Vectorize(
      linalg::LinalgVectorizationOptions options,
      linalg::LinalgTransformationFilter::FilterFunction f = nullptr,
      bool padVectorize = false)
      : Transformation(std::move(f)), options(options),
        vectorizePadding(padVectorize) {}

  Vectorize(StringRef name, linalg::LinalgVectorizationOptions options,
            linalg::LinalgTransformationFilter::FilterFunction f = nullptr,
            bool padVectorize = false)
      : Transformation(std::move(f)), opName(name), options(options),
        vectorizePadding(padVectorize) {}

  void addToPassPipeline(OpPassManager &pm,
                         linalg::LinalgTransformationFilter m) const override {
    pm.addPass(createLinalgStrategyVectorizePass(opName, options, m,
                                                 vectorizePadding));
  }

private:
  std::string opName;
  linalg::LinalgVectorizationOptions options;
  bool vectorizePadding;
};

/// Represent one application of createLinalgStrategyLowerVectorsPass.
struct VectorLowering : public Transformation {
  explicit VectorLowering(
      LinalgVectorLoweringOptions options,
      linalg::LinalgTransformationFilter::FilterFunction f = nullptr)
      : Transformation(std::move(f)), options(options) {}

  void addToPassPipeline(OpPassManager &pm,
                         linalg::LinalgTransformationFilter m) const override {
    pm.addPass(createLinalgStrategyLowerVectorsPass(options, m));
  }

private:
  LinalgVectorLoweringOptions options;
};

/// Codegen strategy controls how a Linalg op is progressively lowered.
struct CodegenStrategy {
  /// Append a pattern to tile the Op `opName` and fuse its producers with
  /// tiling and fusion `options`.
  CodegenStrategy &tileAndFuse(
      StringRef opName, const linalg::LinalgTilingAndFusionOptions &options,
      const linalg::LinalgTransformationFilter::FilterFunction &f = nullptr) {
    transformationSequence.emplace_back(
        std::make_unique<TileAndFuse>(opName, options, f));
    return *this;
  }
  /// Conditionally append a pattern to tile the Op `opName` and fuse its
  /// producers with tiling and fusion `options`.
  CodegenStrategy &tileAndFuseIf(
      bool b, StringRef opName, linalg::LinalgTilingAndFusionOptions options,
      linalg::LinalgTransformationFilter::FilterFunction f = nullptr) {
    return b ? tileAndFuse(opName, std::move(options), std::move(f)) : *this;
  }
  /// Append a pattern to add a level of tiling for Op `opName` with tiling
  /// `options`.
  CodegenStrategy &
  tile(StringRef opName, const linalg::LinalgTilingOptions &options,
       const linalg::LinalgTransformationFilter::FilterFunction &f = nullptr) {
    transformationSequence.emplace_back(
        std::make_unique<Tile>(opName, options, f));
    return *this;
  }
  /// Conditionally append a pattern to add a level of tiling for
  /// `LinalgOpType` with tiling `options`.
  CodegenStrategy &
  tileIf(bool b, StringRef opName, linalg::LinalgTilingOptions options,
         linalg::LinalgTransformationFilter::FilterFunction f = nullptr) {
    return b ? tile(opName, std::move(options), std::move(f)) : *this;
  }
  /// Append a pattern to pad and hoist the operands of Op `opName` with padding
  /// `options`.
  CodegenStrategy &
  pad(StringRef opName, const linalg::LinalgPaddingOptions &options,
      const linalg::LinalgTransformationFilter::FilterFunction &f = nullptr) {
    transformationSequence.emplace_back(
        std::make_unique<Pad>(opName, options, f));
    return *this;
  }
  /// Conditionally append a pattern to pad and hoist the operands of Op
  /// `opName` with padding `options`.
  CodegenStrategy &
  padIf(bool b, StringRef opName, linalg::LinalgPaddingOptions options,
        linalg::LinalgTransformationFilter::FilterFunction f = nullptr) {
    return b ? pad(opName, std::move(options), std::move(f)) : *this;
  }
  /// Append patterns to decompose convolutions.
  CodegenStrategy &decompose(
      const linalg::LinalgTransformationFilter::FilterFunction &f = nullptr) {
    transformationSequence.emplace_back(std::make_unique<Decompose>(f));
    return *this;
  }
  /// Conditionally append patterns to decompose convolutions.
  CodegenStrategy &
  decomposeIf(bool b,
              linalg::LinalgTransformationFilter::FilterFunction f = nullptr) {
    return b ? decompose(std::move(f)) : *this;
  }
  /// Append a pattern to peel 'LinalgOpType'.
  CodegenStrategy &
  peel(StringRef opName, const LinalgPeelOptions &options,
       const linalg::LinalgTransformationFilter::FilterFunction &f = nullptr) {
    transformationSequence.emplace_back(
        std::make_unique<Peel>(opName, options, f));
    return *this;
  }
  /// Conditionally append a pattern to peel 'LinalgOpType'.
  CodegenStrategy &
  peelIf(bool b, StringRef opName, const LinalgPeelOptions &options,
         linalg::LinalgTransformationFilter::FilterFunction f = nullptr) {
    return b ? peel(opName, options, std::move(f)) : *this;
  }
  /// Append a pattern to rewrite `LinalgOpType` as a vector operation.
  CodegenStrategy &vectorize(
      StringRef opName,
      const linalg::LinalgTransformationFilter::FilterFunction &f = nullptr,
      bool vectorizePadding = false) {
    transformationSequence.emplace_back(std::make_unique<Vectorize>(
        opName, linalg::LinalgVectorizationOptions(), f, vectorizePadding));
    return *this;
  }
  /// Conditionally append a pattern to rewrite `LinalgOpType` as a vector
  /// operation.
  CodegenStrategy &
  vectorizeIf(bool b, StringRef opName,
              linalg::LinalgTransformationFilter::FilterFunction f = nullptr,
              bool vectorizePadding = false) {
    return b ? vectorize(opName, std::move(f), vectorizePadding) : *this;
  }
  /// Append a pattern to lower all vector operations.
  CodegenStrategy &vectorLowering(LinalgVectorLoweringOptions options) {
    transformationSequence.emplace_back(
        std::make_unique<VectorLowering>(options));
    return *this;
  }
  /// Configure the post staged-patterns global enabling passes options.
  CodegenStrategy &
  setVectorTransferToSCFOptions(linalg::LinalgEnablingOptions options) {
    linalgEnablingOptions = options;
    return *this;
  }

  /// Apply the transformation patterns in sequence with cleanup
  /// transformations interleaved.
  void configurePassPipeline(OpPassManager &pm, MLIRContext *context,
                             bool addEnablePass = true) const;

private:
  LogicalResult postPatternTransforms(Operation *func) const;

  linalg::LinalgEnablingOptions linalgEnablingOptions;
  SmallVector<std::unique_ptr<Transformation>, 4> transformationSequence;
};

} // namespace LinalgExt
} // namespace IREE
} // namespace iree_compiler
} // namespace mlir

#endif // IREE_DIALECTS_DIALECT_LINALGEXT_TRANSFORMS_CODEGENSTRATEGY_H_
