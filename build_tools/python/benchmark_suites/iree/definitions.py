## Copyright 2022 The IREE Authors
#
# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Defines IREE benchmarks."""

import itertools
from typing import List, Sequence, Tuple

from e2e_test_framework.device_specs import device_collections
from e2e_test_framework.models import model_groups
from e2e_test_framework.definitions import common_definitions, iree_definitions
from e2e_test_framework import unique_ids

MODULE_BENCHMARK_TOOL = "iree-benchmark-module"


def _generate_e2e_model_run_configs(
    module_generation_configs: Sequence[
        iree_definitions.ModuleGenerationConfig],
    module_execution_configs: Sequence[iree_definitions.ModuleExecutionConfig],
    device_specs: Sequence[common_definitions.DeviceSpec],
    input_data: common_definitions.ModelInputData = common_definitions.
    RANDOM_MODEL_INPUT_DATA,
) -> List[iree_definitions.E2EModelRunConfig]:
  """Generates the run configs from the product of compile configs and execution configs."""
  return [
      iree_definitions.E2EModelRunConfig(
          module_generation_config=module_generation_config,
          module_execution_config=module_execution_config,
          target_device_spec=device_spec,
          input_data=input_data) for module_generation_config,
      module_execution_config, device_spec in itertools.product(
          module_generation_configs, module_execution_configs, device_specs)
  ]


class Linux_x86_64_Benchmarks(object):
  """Benchmarks on x86_64 linux devices."""

  CASCADELAKE_CPU_TARGET = iree_definitions.CompileTarget(
      target_architecture=common_definitions.DeviceArchitecture.
      X86_64_CASCADELAKE,
      target_backend=iree_definitions.TargetBackend.LLVM_CPU,
      target_abi=iree_definitions.TargetABI.LINUX_GNU)

  CASCADELAKE_COMPILE_CONFIG = iree_definitions.CompileConfig(
      id=unique_ids.IREE_COMPILE_CONFIG_LINUX_CASCADELAKE,
      tags=["default-flags"],
      compile_targets=[CASCADELAKE_CPU_TARGET])

  @classmethod
  def generate(
      cls
  ) -> Tuple[List[iree_definitions.ModuleGenerationConfig],
             List[iree_definitions.E2EModelRunConfig]]:
    """Generates IREE compile and run configs."""

    default_execution_configs = cls._generate_default_execution_configs()

    module_generation_configs = [
        iree_definitions.ModuleGenerationConfig(
            compile_config=cls.CASCADELAKE_COMPILE_CONFIG, model=model)
        for model in model_groups.SMALL + model_groups.LARGE
    ]
    cascadelake_devices = device_collections.DEFAULT_DEVICE_COLLECTION.query_device_specs(
        architecture=common_definitions.DeviceArchitecture.X86_64_CASCADELAKE,
        platform=common_definitions.DevicePlatform.GENERIC_LINUX)
    e2e_model_run_configs = _generate_e2e_model_run_configs(
        module_generation_configs=module_generation_configs,
        module_execution_configs=default_execution_configs,
        device_specs=cascadelake_devices)

    return (module_generation_configs, e2e_model_run_configs)

  @staticmethod
  def _generate_default_execution_configs(
  ) -> List[iree_definitions.ModuleExecutionConfig]:
    module_execution_configs = [
        iree_definitions.ModuleExecutionConfig(
            id=unique_ids.IREE_MODULE_EXECUTION_CONFIG_LOCAL_SYNC,
            tags=["full-inference", "default-flags"],
            loader=iree_definitions.RuntimeLoader.EMBEDDED_ELF,
            driver=iree_definitions.RuntimeDriver.LOCAL_SYNC,
            tool=MODULE_BENCHMARK_TOOL)
    ]
    for thread_num in [1, 4, 8]:
      module_execution_configs.append(
          iree_definitions.ModuleExecutionConfig(
              id=
              f"{unique_ids.IREE_MODULE_EXECUTION_CONFIG_LOCAL_TASK_BASE}_{thread_num}",
              tags=[f"{thread_num}-thread", "full-inference", "default-flags"],
              loader=iree_definitions.RuntimeLoader.EMBEDDED_ELF,
              driver=iree_definitions.RuntimeDriver.LOCAL_TASK,
              tool=MODULE_BENCHMARK_TOOL,
              extra_flags=[f"--task_topology_group_count={thread_num}"]))
    return module_execution_configs


def generate() -> Tuple[List[iree_definitions.ModuleGenerationConfig],
                        List[iree_definitions.E2EModelRunConfig]]:
  """Generates all compile and run configs for IREE benchmarks."""
  return Linux_x86_64_Benchmarks.generate()
