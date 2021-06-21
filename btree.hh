#pragma once

#include "asm.hh"
#include "cursor.hh"
#include <variant>
#include <array>

struct codealloc {
  codealloc(size_t code_length) : m_code_length(code_length) {
    m_code = (cursor_t)::mmap(NULL, m_code_length,
                              PROT_EXEC | PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANON, -1, 0);
    assert((void*)m_code != MAP_FAILED);

    // If we fall out of bounds immediately return.
    ::memset(m_code, RETQ, code_length);
    m_cursor = m_code;
  }
  ~codealloc() { ::munmap(m_code, m_code_length); }
  void commit() {}
  void put_code(code )
private:
  cursor_t m_code, m_code_lenght, m_cursor;
};




template<typename V>
struct BTreeIndex {
  constexpr size_t min_size() { return 0; }
  constexpr size_t max_size() { return 0; }
  operator new()

  struct node {
    struct subnode { V min; cursor_t child; }
    std::vector<subnode> subnodes;
  }
};
