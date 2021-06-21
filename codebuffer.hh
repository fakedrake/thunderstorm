#pragma once

#include <fmt/format.h>
#include <stdint.h>

#include <string>

#include "cursor.hh"
#include "btree.hh"

template<size_t max_codeblock_bytes,typename Traversal>
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

  bool push_cons(uint64_t car, std::string&& msg) {
    cursor_t end = m_code + m_code_length;
    if (end - m_cursor < max_codeblock_bytes) {
      return false;
    }
    if (m_conses.empty()) {
      m_conses.emplace_front(m_cursor, end, car, std::move(msg));
      auto cursor_tmp = m_conses.front().code_end();
      if (cursor_tmp - m_cursor > max_codeblock_bytes) {
        fmt::print("cursor_tmp: {}, m_cursor: {}, max_bytes: {}, diff: {}\n",
                   (void*)cursor_tmp, (void*)m_cursor, max_codeblock_bytes,
                   cursor_tmp - m_cursor);
        ::abort();
      }
      m_cursor = cursor_tmp;
      return true;
    }
    Traversal<std::string>& cdr = m_conses.front();
    m_conses.emplace_front(m_cursor, end, car, cdr, std::move(msg));
    auto cursor_tmp = m_conses.front().code_end();
    if (cursor_tmp - m_cursor > max_codeblock_bytes) {
      printf("cursor_tmp: %p, m_cursor: %p\n", (void*)(cursor_tmp),
             (void*)(m_cursor));
      ::abort();
    }
    m_cursor = cursor_tmp;
    return true;
  }

  bool put_btree_node(std::vector<std::pair<uint32_t,cursor_t>> vals) {
  }

  Traversal<std::string>* find_cons(uint64_t query) {
    if (m_conses.empty()) return nullptr;
    return m_conses.front().find_cons(query);
  }

 private:
  std::list<Traversal<std::string>> m_conses;
  size_t m_code_length;
  cursor_t m_code, m_cursor;
};
