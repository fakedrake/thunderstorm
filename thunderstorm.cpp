#include <list>
#include <assert.h>
#include <sys/mman.h>

#include <fmt/format.h>
#include <benchmark/benchmark.h>

#include "cursor.hh"
#include "asm.hh"
#include "list.hh"
#include "codebuffer.hh"

#define MAX_CODEBLOCK_BYTES 48
#define START_VALUE 1024
#define END_VALUE (1024 << 13)

// Linear search though an array of pairs.
static void BM_array(benchmark::State& state) {
  srand(42);
  const size_t elems = state.range(0) / sizeof(std::pair<size_t, std::string>);
  std::vector<std::pair<size_t, std::string>> buf(elems);
  for (int i = 0; i < elems; i++) {
    buf.emplace_back(i, fmt::format("Number {}", i));
  }

  // Linear search
  for (auto _ : state) {
    size_t lu_val = rand() % elems;
    for (int i = 0; i < elems; i++) {
      if (buf[i].first == lu_val) {
        benchmark::DoNotOptimize(buf[i].second);
        break;
      }
    }
  }
}

// Linear search via traversal. The elements are not sequential.
static void BM_jit_list_search(benchmark::State& state) {
  size_t elems = 0;
  CodeBuffer<MAX_CODEBLOCK_BYTES,ListSearch> buf(state.range(0));
  while (buf.push_cons(elems, fmt::format("Number {}", elems))) elems++;
  // Perform setup here
  for (auto _ : state) {
    size_t lu_val = rand() % elems;
    benchmark::DoNotOptimize(buf.find_cons(lu_val)->get_data());
  }
}

#define MY_BMARK(x) \
  BENCHMARK(x)->RangeMultiplier(2)->Range(START_VALUE, END_VALUE);

// Run the benchmark
int main(int argc, char** argv) {
  MY_BMARK(BM_jit_list_search);
  MY_BMARK(BM_array);

  // these entries are from BENCHMARK_MAIN
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  ::benchmark::RunSpecifiedBenchmarks();
}
