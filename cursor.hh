#pragma once

typedef unsigned char* cursor_t;
#if 1
inline cursor_t align_cursor(cursor_t c) {
  return reinterpret_cast<cursor_t>(reinterpret_cast<uint64_t>(c + 0xf) &
                                    ~(0xf));
}
#else
inline cursor_t align_cursor(cursor_t c) {
  return c;
}
#endif
