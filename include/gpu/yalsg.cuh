/**
 * @file yalsg.cuh
 * @author Yihao Sun (ysun799@bloomberg.com)
 * @brief GPU version of yals.h, use C++
 * 
 * @version 0.1
 * @date 2025-06-05
 * 
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "yals.h"
#include "yils.h"
#include <cstdint>
#include <rmm/device_vector.hpp>
#include <rmm/mr/device/pool_memory_resource.hpp>
#include <stdint.h>
#include <rmm/exec_policy.hpp>

#define Vec(T) rmm::device_vector<T>
#define D_POLICY rmm::exec_policy()

// macro get the highest 32 bit of u64 number
#define H32_BIT(x) ((x) >> 32)

// macro get the lowest 32 bit of u64 number
#define L32_BIT(x) ((x) & 0xFFFFFFFF)


struct GPUMemoryManager {
  rmm::mr::device_memory_resource * mr;
  // memory pool
  rmm::mr::pool_memory_resource<rmm::mr::cuda_memory_resource> * pool;

  GPUMemoryManager();
  ~GPUMemoryManager();
};

// Yals on Device
struct Yalsg {

  Yalsg(Yals * yals) : yals(yals) {}

  // Host version of Yals
  Yals * yals;

  // current unsatisfied clauses
  // This vector has the same size as the number of total clauses
  Vec(int) unsat;

  // the refs are Index of occur-list, lit ↦ clauses;
  Vec(int) refs;

  // The flips array in YALSAT is used to track the number of times
  // each variable has been flipped during the local search process.
  Vec(uint32_t) flips;

  // TODO: mark vector
  
  // the value assigned to each variable
  Vec(Word) vals;
  // the variable assignment that has achieved the minimum number of unsatisfied clauses so far
  Vec(Word) best;
  // the current variable assignment
  Vec(Word) tmp;
  // mask for clearing bits in the variable assignments
  // all bits are set to true initially
  // Works in conjunction with the set array to manage unit clauses and forced variable assignments
  // Used in yals_set_units() to apply unit clause constraints to the current variable assignment
  // Helps maintain the constraints from unit clauses during the solving process
  Vec(Word) clear;
  Vec(Word) sets;
  
  // clauses database
  Vec(int) cdb;

  // cached assignments
  Vec(Word) cache;

  // minimum unsatisfied clauses of each cached clauses
  Vec(int) mins;
  
  // count the number of satisfied clauses for each variable
  Vec(uint32_t) satcnt;

  // Occurrence list
  Vec(int) occs;
  Vec(uint32_t) occs_len;

  // weights for each variable, used in ProbSAT
  Vec(uint32_t) weights;
  Vec(uint32_t) weightedbreak;

  // vector of critical variables for each clause
  Vec(int) crit;

  Vec(uint32_t) breaks;

  // scores for each cached clause
  Vec(double) scores;

  // candidates for each cached clause
  Vec(int) cands;
};




