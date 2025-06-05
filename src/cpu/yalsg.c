

#include "yals_ext.h"
#include "gpu/gpu.h"


void yals_init_gpu(Yals * yals) {
  if (!yals) return;
  // init GPU memory manager
  yals->gpu_memory_manager = gpu_memory_manager_new();
  yals->yalsg = yalsg_new(yals);
}

