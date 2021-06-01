#include <stdint.h>
#include <stddef.h>

#define unlikely(i) __builtin_expect(i, 0)

extern size_t array[];
extern size_t check_val_f;
extern size_t check_val_g;
extern size_t return_val_f;
extern size_t return_val_g;
extern size_t return_null_g;


// The argument is stored in %rdi
// The return is always in %rax
//

size_t __attribute__((noinline)) g(size_t i) {
  if (unlikely(array[i] == check_val_g)) {
    return return_val_g;
  }
  return return_null_g;
}


/*
f:
	movq	check_val_f@GOTPCREL(%rip), %rax
	cmpq	%rdi, (%rax)
	je	.return_label
	addq	$1, %rdi
	jmp	g                   # TAILCALL
.return_label:
	movq	return_val_f@GOTPCREL(%rip), %rax
	movq	(%rax), %rax
	retq
*/
extern size_t f(size_t i) {
  if (unlikely(i == check_val_f)) {
    return return_val_f;
  }
  return g(i + 1) + 1;
}
