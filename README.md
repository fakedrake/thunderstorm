# Thunderstorm: JIT traversals

The main idea is that when we build a data structure, it is often the case that the main workload on it is traversal. Instead of trying to make a compact datastructure to improve locality we generate executable assembly that corresponds to the traversal itself. The hope is that the processor will be much better at prefetching instructions behind immediate jumps than data behind a pointer dereference. In other words we produce JIT traversals of a datastructure instead of (or in addition to) the datastructure itself.

As an minimum POC we implemented a linked list JIT traversal and we compare linear search in the list with linear search over a contiguous array. We notice the the JIT traversal is faster than array linear search as long as the entire traversal fits in L2.

In the following table:

- `BM_array` is linear search in an array.
- `BM_codebuffer` is the linked list JIT traversal.
- The size of the JIT traversal is always a few times larger than that of the array.
- `BM_array/abc` and `BM_codebuffer/1024` work on a datastructure that has the **same** number of elements (1024 / 40). The size of the JIT traversal (applicable only in the case of `BM_codebuffer`) is 1024 bytes. 

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
BM_array/131072             2413 ns         2413 ns       288632 // BM_array starts winning
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
BM_codebuffer/131072        1256 ns         1256 ns       533786 // BM_array starts winning
BM_codebuffer/262144       10859 ns        10858 ns        73264
BM_codebuffer/524288       37169 ns        37166 ns        18714
BM_codebuffer/1048576      99209 ns        99197 ns         6902
BM_codebuffer/2097152     216900 ns       216889 ns         3177
BM_codebuffer/4194304     451893 ns       451835 ns         1554
BM_codebuffer/6291456     753728 ns       753683 ns          832
```

## How to run

**THIS ONLY RUNS ON x86_64**. You will need [nix](https://nixos.org/) to run the benchmarks. You can get it by running

```
$ curl -L https://nixos.org/nix/install | sh
```

Once nix is installed

```
$ nix-shell 
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./thunderstorm
```


## Roadmap

This is the most naive approach. There are a few optimizations that come to mind:

- Cache-align the ends of jumps (might need to pad with multibyte nops)
- Larger objects being checked for equality would allow us to use SIMD instructions and mitigate the size ratio (at the moment for every 8 byte key + 8byte address to the value we use a total of about 31 bits of instructions. For larger keys we could strike a better ratio.
- This approach uses the instruction prefetching mechanisms which are unlikely to reach the memory bus capacity. Investigate using prefetch/load instructions to complement.
- These experiments have a clear boundary between JIT code being driven by normal code. It would be interesting to a graph representation that would allow for extremely fast BFS where the neighbor sets are stored as code that generates frontier fragments if they are not visited. The frontier is a JIT traversal that calls on the frontier fragment generators. Work for each thead must be statically scheduled. 
