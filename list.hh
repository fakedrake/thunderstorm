#pragma once

#include <stdint.h>
#include "cursor.hh"


template <typename T>
class ListSearch {
 public:
  ListSearch() = delete;
  ListSearch(const ListSearch&) = delete;
  ListSearch(ListSearch&&) = delete;
  ListSearch(cursor_t code, cursor_t end, uint64_t car, ListSearch<T>& cdr,
            T&& data)
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
    c = op_retq(c);
    m_code_end = align_cursor(c);
    if (m_code_end > end) ::abort();
  }
  ListSearch(cursor_t code, cursor_t end, uint64_t car, T&& data)
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
    c = op_rbx_store(c, reinterpret_cast<uint64_t>(this));
    // return;
    c = op_retq(c);
    m_code_end = align_cursor(c);
    if (m_code_end > end) ::abort();
  }

  ~ListSearch() {}

  ListSearch<T>* find_cons(uint64_t query) {
    return call_asm_rbx_io<ListSearch<T>>(m_code_begin, query);
  }

  cursor_t code_end() const { return m_code_end; }

  T& get_data() { return m_data; }
  size_t size() const { return m_code_end - m_code_begin; }

 private:
  cursor_t m_code_begin, m_code_end, m_car_op, m_cdr_op;
  T m_data;
};
