# YalGSAT

This is yet another GPU based local search SAT solver. It is a fork of [YalSAT](https://github.com/yalsat/yalsat) with GPU support.
We notice that for large random SAT instances, the time is mostly spent on `yals_make_clauses_after_flipping_lit` and `yals_break_clauses_after_flipping_lit` (>90% of running time). To be more precise, the function `yals_incsatcnt` and `yals_decsatcnt` are the most time consuming functions called by these two functions. These two functions are memory operation intensive, so it's reasonable to migrate them to GPU. The hardest part is on data structure design, we need change the original queue/stack (doesn't necessarily include any critical clause, can be fully lock-free) to a more parallelizable data structure.

## Build

```bash
# for H100 GPU
cmake -Bbuild -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CUDA_ARCHITECTURES=90 .
cd build && make -j
```

## CHANGES

- 2025-06-05: Migrated Project to CMake, prepared for RAPIDS/CUDA integration.


## TODO

Milestone 1:
Migrate iterative local search to GPU.
- [ ] Migrate everything on pick_clause to GPU
- [ ] Migrate everything on pick_literal to GPU
- [ ] Migrate everything on make_clauses_after_flipping_lit to GPU
- [ ] Migrate everything on break_clauses_after_flipping_lit to GPU
- [ ] Migrate everything on flip to GPU

Milestone 2:
Enable restart of inner/outer loop.

Milestone 3:
Potofilo style local search, boot miltple stream for different restarts.
