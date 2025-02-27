// RUN: iree-opt --split-input-file --iree-hal-conversion %s | FileCheck %s

// CHECK-LABEL: util.global private mutable @rwTimepoint : !hal.fence
util.global private mutable @rwTimepoint = #stream.timepoint<immediate>
// CHECK: func.func @globalTimepoint(%arg0: !hal.fence) -> !hal.fence
func.func @globalTimepoint(%arg0: !stream.timepoint) -> !stream.timepoint {
  // CHECK: util.global.store %arg0, @rwTimepoint
  util.global.store %arg0, @rwTimepoint : !stream.timepoint
  // CHECK: %[[VALUE:.+]] = util.global.load @rwTimepoint
  %value = util.global.load @rwTimepoint : !stream.timepoint
  // CHECK: return %[[VALUE]]
  return %value : !stream.timepoint
}

// -----

// CHECK-LABEL: @timepointImmediate
func.func @timepointImmediate() -> !stream.timepoint {
  // CHECK: %[[FENCE:.+]] = util.null : !hal.fence
  %0 = stream.timepoint.immediate => !stream.timepoint
  // CHECK: return %[[FENCE]]
  return %0 : !stream.timepoint
}

// -----

// CHECK-LABEL: @timepointImportFence
func.func @timepointImportFence(%arg0: !hal.fence) -> !stream.timepoint {
  %0 = stream.timepoint.import %arg0 : (!hal.fence) => !stream.timepoint
  // CHECK: return %arg0
  return %0 : !stream.timepoint
}

// -----

// CHECK-LABEL: @timepointExportFence
func.func @timepointExportFence(%arg0: !stream.timepoint) -> !hal.fence {
  %0 = stream.timepoint.export %arg0 => (!hal.fence)
  // CHECK: return %arg0
  return %0 : !hal.fence
}

// -----

// CHECK-LABEL: @timepointJoin
func.func @timepointJoin(%arg0: !stream.timepoint, %arg1: !stream.timepoint) -> !stream.timepoint {
  // CHECK: %[[FENCE:.+]] = hal.fence.join at([%arg0, %arg1]) -> !hal.fence
  %0 = stream.timepoint.join max(%arg0, %arg1) => !stream.timepoint
  // CHECK: return %[[FENCE]]
  return %0 : !stream.timepoint
}

// -----

// CHECK-LABEL: @timepointAwait
func.func @timepointAwait(%arg0: !stream.timepoint, %arg1: !stream.resource<staging>, %arg2: !stream.resource<*>) -> (!stream.resource<staging>, !stream.resource<*>) {
  %c100 = arith.constant 100 : index
  %c200 = arith.constant 200 : index
  // CHECK: %[[WAIT_OK:.+]] = hal.fence.await until([%arg0]) timeout_millis(%c-1_i32) : i32
  // CHECK-NEXT: util.status.check_ok %[[WAIT_OK]]
  %0:2 = stream.timepoint.await %arg0 => %arg1, %arg2 : !stream.resource<staging>{%c100}, !stream.resource<*>{%c200}
  // CHECK: return %arg1, %arg2
  return %0#0, %0#1 : !stream.resource<staging>, !stream.resource<*>
}
