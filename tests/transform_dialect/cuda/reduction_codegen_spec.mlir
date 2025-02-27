// RUN: iree-opt %s

transform.structured.canonicalized_sequence failures(propagate) {
^bb1(%variant_op: !pdl.operation):
  %fill = transform.structured.match ops{["linalg.fill"]} in %variant_op

  // Split the reduction by 2 to obtain a more meaty parallel op with
  // parallelism across size(reduction) / 2 threads.
  %0 = transform.structured.match ops{["linalg.generic"]} in %variant_op
  %init_or_alloc_op, %more_parallel_fill_op, %more_parallel_op, %combiner_op =
    transform.structured.split_reduction %0
      { split_factor = 2, insert_split_dimension = 1 }

  // First level of tiling + fusion parallelizes to blocks.
  // The mapping to block ids can only happen after bufferization atm.
  %foreach_thread_grid, %grid_combiner_op =
    transform.iree.tile_to_foreach_thread_and_workgroup_count_region %combiner_op tile_sizes [1]
  %not_combiner = transform.merge_handles %fill, %more_parallel_fill_op, %more_parallel_op
  transform.structured.fuse_into_containing_op %not_combiner into %foreach_thread_grid

  // Second level of tiling + fusion parallelizes to threads.
  // The mapping to thread ids can only happen after bufferization atm.
  %fill_2d = transform.structured.match ops{["linalg.fill"]} filter_result_type = tensor<1x2xf32> in %variant_op

  %grid_more_parallel_op = transform.structured.match interface{LinalgOp}
    attributes{iterator_types = ["parallel", "parallel", "reduction"]} in %variant_op
  %foreach_thread_block_more_parallel_op, %block_more_parallel_op =
    transform.structured.tile_to_foreach_thread_op %grid_more_parallel_op tile_sizes [1, 1, 0] (mapped to dims [2, 1, 0])
  transform.structured.fuse_into_containing_op %fill_2d into %foreach_thread_block_more_parallel_op
  
  // Second level of tiling + fusion parallelizes to threads.
  // The mapping to thread ids can only happen after bufferization atm.
  %fill_1d = transform.structured.match ops{["linalg.fill"]} filter_result_type = tensor<1xf32> in %variant_op
  %foreach_thread_block_combiner_op, %block_combiner_op =
    transform.structured.tile_to_foreach_thread_op %grid_combiner_op tile_sizes [1, 0, 0] (mapped to dims [2, 1, 0])
  transform.structured.fuse_into_containing_op %fill_1d into %foreach_thread_block_combiner_op

  %func = transform.structured.match ops{["func.func"]} in %variant_op
  %func_2 = transform.iree.apply_patterns %func { rank_reducing }
  %func_3 = transform.structured.vectorize %func_2

  %variant_op_2 = transform.iree.bufferize { target_gpu } %variant_op
  %func_4 = transform.structured.match ops{["func.func"]} in %variant_op_2

  %func_5 = transform.iree.foreach_thread_to_workgroup %func_4
  %func_6 = transform.iree.map_nested_foreach_thread_to_gpu_threads %func_5
      { workgroup_size = [32, 2, 1] }

  // Vector distribution needs to happen on buffers.
  %func_7 = transform.iree.apply_patterns %func_6 { rank_reducing }
  %if_op = transform.structured.match ops{["scf.if"]} in %variant_op_2
  %warp = transform.iree.vector.to_warp_execute_on_lane_0 %if_op { warp_size = 32 }
  transform.iree.vector.warp_distribute %func_7
}
