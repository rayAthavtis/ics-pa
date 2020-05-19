#include "common.h"
#include "syscall.h"

uintptr_t sys_none() {
  return 1;
}

void sys_exit(int code) {
  _halt(code);
}

ssize_t sys_write(int fd, uintptr_t buf, size_t len) {
  Log("output: %s", (char *)buf);
  if (fd==1 || fd==2) {
    char ch;
	for (int i=0; i<len; i++) {
	  memcpy(&ch, (void *)buf+i, 1);
	  _putc(ch);
	}
	return len;
  }
  return -1;
}

int sys_brk(uintptr_t addr) {
  return 0;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
	case SYS_none: SYSCALL_ARG1(r) = sys_none(); break;
	case SYS_exit: sys_exit(0); break;
	case SYS_write: SYSCALL_ARG1(r) = sys_write(a[1], a[2], a[3]); break;
	case SYS_brk: SYSCALL_ARG1(r) = sys_brk(a[1]); break;
	
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
