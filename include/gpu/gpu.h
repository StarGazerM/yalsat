/**
 * @file gpu.h
 * @author Yihao Sun (ysun799@bloomberg.com)
 * @brief C API for GPU version of YALS, wrapper for yalsg.cuh
 * 
 * @version 0.1
 * @date 2025-06-05
 * 
 * @copyright Copyright (c) 2025
 */

#pragma once

#include "yals.h"

// Type erasure for Yalsg

#ifdef __cplusplus
extern "C" {
#endif

// type erasure of RMM memory manager
typedef struct GPUMemoryManager GPUMemoryManager;

GPUMemoryManager * gpu_memory_manager_new();

typedef struct Yalsg Yalsg;

Yalsg * yalsg_new(Yals * yals);

void yalsg_load_to_device(Yalsg * yalsg);

void yalsg_flip(Yalsg * yalsg);

int yalsg_pick_clause(Yalsg * yalsg);
int yalsg_pick_literal(Yalsg * yalsg, int cidx);


// These two functions are most time consuming
void yalsg_make_clauses_after_flipping_lit(Yalsg * yalsg, int lit);
void yalsg_break_clauses_after_flipping_lit(Yalsg * yalsg, int lit);

#ifdef __cplusplus
}
#endif



