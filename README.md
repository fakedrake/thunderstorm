# Thunderstorm: JIT traversals

We benchmark the traversal of a linked list. It looks like if the
codebuffer fits in L2 the JIT traversal is faster than a linear search
through an array.


```
Running ./build/thunderstorm
Run on (4 X 3600 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 6144 KiB (x1)
Load Average: 0.93, 0.59, 0.41
----------------------------------------------------------------
Benchmark                      Time             CPU   Iterations
----------------------------------------------------------------
BM_array/1024               34.0 ns         34.0 ns     20406072
BM_array/2048               52.9 ns         52.9 ns     12770383
BM_array/4096               87.7 ns         87.7 ns      7846345
BM_array/8192                156 ns          156 ns      4432260
BM_array/16384               296 ns          296 ns      2335146
BM_array/32768               582 ns          582 ns      1147466
BM_array/65536              1219 ns         1219 ns       569313
BM_array/131072             2413 ns         2413 ns       288632
BM_array/262144             4916 ns         4916 ns       136364
BM_array/524288            10517 ns        10516 ns        66019
BM_array/1048576           21160 ns        21159 ns        32335
BM_array/2097152           42660 ns        42657 ns        16254
BM_array/4194304           93018 ns        93012 ns         7474
BM_array/6291456          185729 ns       185712 ns         3667
BM_codebuffer/1024          29.5 ns         29.5 ns     23262774
BM_codebuffer/2048          34.0 ns         34.0 ns     20511977
BM_codebuffer/4096          43.1 ns         43.1 ns     16388015
BM_codebuffer/8192          67.6 ns         67.6 ns     10142880
BM_codebuffer/16384          133 ns          133 ns      5208699
BM_codebuffer/32768          279 ns          279 ns      2543292
BM_codebuffer/65536          552 ns          552 ns      1210726
BM_codebuffer/131072        1256 ns         1256 ns       533786
BM_codebuffer/262144       10859 ns        10858 ns        73264
BM_codebuffer/524288       37169 ns        37166 ns        18714
BM_codebuffer/1048576      99209 ns        99197 ns         6902
BM_codebuffer/2097152     216900 ns       216889 ns         3177
BM_codebuffer/4194304     451893 ns       451835 ns         1554
BM_codebuffer/6291456     753728 ns       753683 ns          832
```
