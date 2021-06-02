#include <benchmark/benchmark.h>

#include <list>
#include <assert.h>
#include <sys/mman.h>
#include <fmt/format.h>

typedef unsigned char* cursor_t;

#define REL_ADDR(instr,target) ((instr) + 1 - (target))
inline cursor_t op_jmp(cursor_t c, cursor_t target) {
  size_t rel32 = target - &c[5];
  c[0] = 0xe9; // 0xea would be a far jump. near jumps are predicted.
  c[1] = rel32 & 0xff;
  c[2] = (rel32 >> 8) & 0xff;
  c[3] = (rel32 >> 16) & 0xff;
  c[4] = (rel32 >> 24) & 0xff;
  return &c[5];
}

inline cursor_t op_jump_equal(cursor_t c, cursor_t target) {
  c[0] = 0x74;
  c[1] = target - &c[2];
  return &c[2];
}

inline cursor_t op_rax_store(cursor_t c, uint64_t imm64) {
  c[0] = 0x48;
  c[1] = 0xb8;
  c[2] = imm64 & 0xff;
  c[3] = (imm64 >> 8) & 0xff;
  c[4] = (imm64 >> 16) & 0xff;
  c[5] = (imm64 >> 24) & 0xff;
  c[6] = (imm64 >> 32) & 0xff;
  c[7] = (imm64 >> 40) & 0xff;
  c[8] = (imm64 >> 48) & 0xff;
  c[9] = (imm64 >> 56) & 0xff;
  return &c[10];
}

inline cursor_t op_rbx_store(cursor_t c, uint64_t imm64) {
  c[0] = 0x48;
  c[1] = 0xbb;
  c[2] = imm64 & 0xff;
  c[3] = (imm64 >> 8) & 0xff;
  c[4] = (imm64 >> 16) & 0xff;
  c[5] = (imm64 >> 24) & 0xff;
  c[6] = (imm64 >> 32) & 0xff;
  c[7] = (imm64 >> 40) & 0xff;
  c[8] = (imm64 >> 48) & 0xff;
  c[9] = (imm64 >> 56) & 0xff;
  return &c[10];
}

// UD
inline cursor_t op_invalid(cursor_t c) {
  c[0] = 0x0f;
  c[1] = 0x0b;
  return &c[2];
}

inline cursor_t op_cmp_rax_rbx(cursor_t c) {
  c[0] = 0x48;
  c[1] = 0x39;
  c[2] = 0xc3;
  return &c[3];
}

inline cursor_t op_mov_rax_rbx(cursor_t c) {
  c[0] = 0x48;
  c[1] = 0x89;
  c[2] = 0xc3;
  return &c[3];
}


#define RETQ 0xc3
inline cursor_t op_retq(cursor_t c) {
  c[0] = RETQ;
  return &c[1];
}

// Call call a function that uses %rbx as IO and clobbers %rax.
template <typename T>
inline T* call_asm_rbx_io(cursor_t c, uint64_t arg) {
  assert(*c);
  size_t ret;
  __asm__("movq %[arg], %%rbx\n"
          "call *%[cursor]\n"
          "movq %%rbx, %[ret]\n"
          : [ret]"=r"(ret)              /* output */
          : [cursor]"r"(c), [arg]"r"(arg)       /* input */
          : "%rax", "%rbx", "%rcx" /* clobbered registers */
  );
  return reinterpret_cast<T*>(ret);
}

template <typename T>
class Traversal {
 public:
  Traversal() = delete;
  Traversal(const Traversal&) = delete;
  Traversal(Traversal&&) = delete;
  Traversal(cursor_t code, uint64_t car, Traversal<T>& cdr, T&& data)
      : m_data(std::move(data)), m_code_begin(code) {
    assert(cdr.m_code_begin != code);
    cursor_t c = code;
    // rax <- $car;
    c = op_rax_store(c, car);
    // if (rax == car)
    m_car_op = c = op_cmp_rax_rbx(c);
    cursor_t escape_op = c;
    //   goto ___to be filled:escape label___;
    c = op_jump_equal(c, 0);  // dummy
    // goto cdr;
    m_cdr_op = c = op_jmp(c, cdr.m_code_begin);

    cursor_t escape_label = c;
    op_jump_equal(escape_op, escape_label);

    // rbx <- this
    c = op_rbx_store(c, (uint64_t)(this));
    // return;
    m_code_end = op_retq(c);
  }
  Traversal(cursor_t code, uint64_t car, T&& data)
      : m_data(std::move(data)), m_code_begin(code) {
    cursor_t c = code;
    // rax <- $car;
    c = op_rax_store(c, car);
    // if (rax == car)
    m_car_op = c = op_cmp_rax_rbx(c);
    cursor_t escape_op = c;
    c = op_jump_equal(c, 0);  // dummy

    // Return
    c = op_rbx_store(c, 0);
    c = op_retq(c);

    cursor_t escape_label = c;
    op_jump_equal(escape_op, escape_label);

    // rbx <- this
    c = op_rbx_store(c, (uint64_t)(this));
    // return;
    m_code_end = op_retq(c);
  }

  ~Traversal() {}

  Traversal<T>* find_cons(uint64_t query) {
    return call_asm_rbx_io<Traversal<T>>(m_code_begin, query);
  }

  cursor_t code_end() const { return m_code_end; }

  T& get_data() { return m_data; }
  size_t size() const { return m_code_end - m_code_begin; }

 private:
  cursor_t m_code_begin, m_code_end, m_car_op, m_cdr_op;
  T m_data;
};

class CodeBuffer {
 public:
  CodeBuffer(size_t code_length) : m_code_length(code_length) {
    m_code = (cursor_t)::mmap(NULL, m_code_length,
                              PROT_EXEC | PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANON, -1, 0);
    assert((void*)m_code != MAP_FAILED);

    // If we fall out of bounds immediately return.
    ::memset(m_code, RETQ, code_length);
    m_cursor = m_code;
  }
  ~CodeBuffer() { ::munmap(m_code, m_code_length); }

  void push_cons(uint64_t car, std::string&& msg) {
    if (conses.empty()) {
      conses.emplace_front(m_cursor, car, std::move(msg));
      m_cursor = conses.front().code_end();
      return;
    }
    Traversal<std::string>& cdr = conses.front();
    conses.emplace_front(m_cursor, car, cdr, std::move(msg));
    m_cursor = conses.front().code_end();
  }

  Traversal<std::string>* find_cons(uint64_t query) {
    return conses.front().find_cons(query);
  }

 private:
  std::list<Traversal<std::string>> conses;
  size_t m_code_length;
  cursor_t m_code, m_cursor;
};

#define MAX_CODEBLOCK_BYTES 40

#define MY_BMARK(x) \
  BENCHMARK(x)->RangeMultiplier(2)->Range(1024, 1024 * 1024 * 6)

// Linear search though an array of pairs.
static void BM_array(benchmark::State& state) {
  srand(42);
  const size_t elems = state.range(0) / MAX_CODEBLOCK_BYTES;
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
MY_BMARK(BM_array);

// Linear search via traversal. The elements are not sequential.
static void BM_codebuffer(benchmark::State& state) {
  srand(42);
  const size_t elems = state.range(0) / MAX_CODEBLOCK_BYTES;
  CodeBuffer buf(state.range(0));
  for (int i = 0; i < elems; i++) {
    buf.push_cons(i, fmt::format("Number {}", i));
  }

  // Perform setup here
  for (auto _ : state) {
    benchmark::DoNotOptimize(
        buf.find_cons(rand() % elems)->get_data());
  }
}
MY_BMARK(BM_codebuffer);

// Run the benchmark
BENCHMARK_MAIN();
