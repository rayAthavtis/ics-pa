#include "common.h"
#include "syscall.h"

uintptr_t sys_none() {
  return 1;
}

void sys_exit(int code) {
  _halt(code);
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  // a[1] = SYSCALL_ARG2(r);
  // a[2] = SYSCALL_ARG3(r);
  // a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
	case SYS_none: SYSCALL_ARG1(r) = sys_none(); break;
	case SYS_exit: sys_exit(0); break;
	
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
