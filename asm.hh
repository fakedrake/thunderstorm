#pragma once

#define REL_ADDR(instr,target) ((instr) + 1 - (target))
inline cursor_t op_jmp(cursor_t c, cursor_t target) {
  assert(target);
  int rel32 = target - &c[5];
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
