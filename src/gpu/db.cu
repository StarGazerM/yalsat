/**
 * @file db.cu
 * @author Yihao Sun (ysun799@bloomberg.com)
 * @brief GPU clause database
 *
 * This file is a GPU version of struct Yals in yals.c
 *
 * @version 0.1
 * @date 2025-06-05
 *
 * @copyright Copyright (c) 2025
 */

#include "gpu/gpu.h"
#include "gpu/yalsg.cuh"
#include "rmm/device_vector.hpp"
#include "yils.h"

#include "rmm/mr/device/per_device_resource.hpp"
#include <cstdint>
#include <rmm/mr/device/cuda_memory_resource.hpp>

#include <stdint.h>
#include <thrust/host_vector.h>
#include <thrust/for_each.h>
#include <thrust/execution_policy.h>

GPUMemoryManager::GPUMemoryManager() {
  mr = new rmm::mr::cuda_memory_resource();
  auto initial_size = rmm::percent_of_free_device_memory(25);
  pool = new rmm::mr::pool_memory_resource<rmm::mr::cuda_memory_resource>(
      mr, initial_size);
  rmm::mr::set_current_device_resource(pool);
}

GPUMemoryManager *gpu_memory_manager_new() { return new GPUMemoryManager(); }

Yalsg *yalsg_new(Yals *yals) { return new Yalsg(yals); }

void yalsg_load_to_device(Yalsg *yalsg) {
  if (!yalsg->yals) {
    throw std::runtime_error("Yalsg is not initialized");
  }

  yalsg->unsat.resize(yalsg->yals->nvars);
  if (yalsg->yals->unsat.usequeue) {
    // throw error for unimplemented queue
    throw std::runtime_error("Queue is not implemented on GPU");
  } else {
    int cnt = SIZE(yalsg->yals->unsat.stack);
    cudaMemcpy(yalsg->unsat.data().get(), yalsg->yals->unsat.stack.start,
               cnt * sizeof(int), cudaMemcpyHostToDevice);
  }

  // load occ stack into device
  yalsg->occs.resize(yalsg->yals->noccs);
  cudaMemcpy(yalsg->occs.data().get(), yalsg->yals->occs,
             yalsg->yals->noccs * sizeof(int), cudaMemcpyHostToDevice);

  // load refs into device, unlike host refs, device refs will also store the
  // size of each occ stack bucket
  yalsg->refs.resize(2 * yalsg->yals->nvars);
  cudaMemcpy(yalsg->refs.data().get(), yalsg->yals->refs,
             2 * yalsg->yals->nvars * sizeof(int), cudaMemcpyHostToDevice);
  yalsg->occs_len.resize(2 * yalsg->yals->nvars);
  cudaMemcpy(yalsg->occs_len.data().get(), yalsg->yals->occs_bucket_sizes,
             2 * yalsg->yals->nvars * sizeof(uint32_t), cudaMemcpyHostToDevice);
  
  // locad flips into device
  yalsg->flips.resize(yalsg->yals->nvars);
  cudaMemcpy(yalsg->flips.data().get(), yalsg->yals->flips,
             yalsg->yals->nvars * sizeof(uint32_t), cudaMemcpyHostToDevice);

  // load vals into device
  yalsg->vals.resize(yalsg->yals->nvarwords);
  cudaMemcpy(yalsg->vals.data().get(), yalsg->yals->vals,
             yalsg->yals->nvarwords * sizeof(Word), cudaMemcpyHostToDevice);
  // words
  // load best into device
  yalsg->best.resize(yalsg->yals->nvarwords);
  cudaMemcpy(yalsg->best.data().get(), yalsg->yals->best,
             yalsg->yals->nvarwords * sizeof(Word), cudaMemcpyHostToDevice);
  // load tmp into device
  yalsg->tmp.resize(yalsg->yals->nvarwords);
  cudaMemcpy(yalsg->tmp.data().get(), yalsg->yals->tmp,
             yalsg->yals->nvarwords * sizeof(Word), cudaMemcpyHostToDevice);
  // load clear into device
  yalsg->clear.resize(yalsg->yals->nvarwords);
  cudaMemcpy(yalsg->clear.data().get(), yalsg->yals->clear,
             yalsg->yals->nvarwords * sizeof(Word), cudaMemcpyHostToDevice);
  // load sets into device
  yalsg->sets.resize(yalsg->yals->nvarwords);
  cudaMemcpy(yalsg->sets.data().get(), yalsg->yals->set,
             yalsg->yals->nvarwords * sizeof(Word), cudaMemcpyHostToDevice);
  
  auto size_of_cdb = SIZE(yalsg->yals->cdb);
  yalsg->cdb.resize(size_of_cdb);
  cudaMemcpy(yalsg->cdb.data().get(), yalsg->yals->cdb.start,
             size_of_cdb * sizeof(int), cudaMemcpyHostToDevice);
  // load cache into device
  // yalsg->cache.resize(yalsg->yals->cachesizetarget);
  // cudaMemcpy(yalsg->cache.data().get(), yalsg->yals->cache,
  //            yalsg->yals->cachesizetarget * sizeof(Word), cudaMemcpyHostToDevice);
  
  // load mins into device
  auto size_of_mins = SIZE(yalsg->yals->mins);
  yalsg->mins.resize(size_of_mins);
  cudaMemcpy(yalsg->mins.data().get(), yalsg->yals->mins.start,
             size_of_mins * sizeof(int), cudaMemcpyHostToDevice);
  
  // load satcnt into device
  yalsg->satcnt.resize(yalsg->yals->nvars);
  cudaMemcpy(yalsg->satcnt.data().get(), yalsg->yals->satcnt4,
             yalsg->yals->nvars * sizeof(uint32_t), cudaMemcpyHostToDevice);

  // load weights into device
  yalsg->weights.resize(yalsg->yals->nvars);
  cudaMemcpy(yalsg->weights.data().get(), yalsg->yals->weights,
             yalsg->yals->nvars * sizeof(uint32_t), cudaMemcpyHostToDevice);
  // load weightedbreak into device
  yalsg->weightedbreak.resize(yalsg->yals->nvars);
  cudaMemcpy(yalsg->weightedbreak.data().get(), yalsg->yals->weightedbreak,
             yalsg->yals->nvars * sizeof(uint32_t), cudaMemcpyHostToDevice);
  
  // load crit into device
  yalsg->crit.resize(yalsg->yals->nclauses);
  cudaMemcpy(yalsg->crit.data().get(), yalsg->yals->crit,
             yalsg->yals->nclauses * sizeof(int), cudaMemcpyHostToDevice);
  
  // load breaks into device
  auto size_of_breaks = SIZE(yalsg->yals->breaks);
  yalsg->breaks.resize(size_of_breaks);
  cudaMemcpy(yalsg->breaks.data().get(), yalsg->yals->breaks.start,
             size_of_breaks * sizeof(uint32_t), cudaMemcpyHostToDevice);
  
  // load scores into device
  auto size_of_scores = SIZE(yalsg->yals->scores);
  yalsg->scores.resize(size_of_scores);
  cudaMemcpy(yalsg->scores.data().get(), yalsg->yals->scores.start,
             size_of_scores * sizeof(double), cudaMemcpyHostToDevice);
  
  // load cands into device
  auto size_of_cands = SIZE(yalsg->yals->cands);
  yalsg->cands.resize(size_of_cands);
  cudaMemcpy(yalsg->cands.data().get(), yalsg->yals->cands.start,
             size_of_cands * sizeof(int), cudaMemcpyHostToDevice);
  
  // TODO: check all cache related vectors ds
}
