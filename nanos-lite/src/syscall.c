#include "common.h"
#include "syscall.h"

extern int fs_open(const char *pathname, int flags, mode_t mode);
extern ssize_t fs_read(int fd, void *buf, size_t count);
extern ssize_t fs_write(int fd, void *buf, size_t count);
extern off_t fs_lseek(int fd, off_t offset, int whence);
extern int fs_close(int fd);

uintptr_t sys_none() {
  return 1;
}

void sys_exit(int code) {
  _halt(code);
}

ssize_t sys_write(int fd, uintptr_t buf, size_t len) {
  // Log("output: %s", buf);
  if (fd==1 || fd==2) {
    char ch;
	for (int i=0; i<len; i++) {
	  memcpy(&ch, (void *)buf+i, 1);
	  _putc(ch);
	}
	return len;
  }
  if (fd>2) {
    return fs_write(fd, (void *)buf, len);
  }
  return -1;
}

int sys_brk(uintptr_t addr) {
  // Log("sys_brk");
  return 0;
}

int sys_open(uintptr_t pathname, int flags, mode_t mode) {
  return fs_open((char *)pathname, flags, mode);
}

ssize_t sys_read(int fd, uintptr_t buf, size_t len) {
  return fs_read(fd, (void *)buf, len);
}

off_t sys_lseek(int fd, off_t offset, int whence) {
  return fs_lseek(fd, offset, whence);
}

int sys_close(int fd) {
  return fs_close(fd);
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
	case SYS_none: SYSCALL_ARG1(r) = sys_none(); break;
	case SYS_exit: sys_exit(a[1]); break;
	case SYS_write: SYSCALL_ARG1(r) = sys_write(a[1], a[2], a[3]); break;
	case SYS_brk: SYSCALL_ARG1(r) = sys_brk(a[1]); break;
	case SYS_open: SYSCALL_ARG1(r) = sys_open(a[1], a[2], a[3]); break;
	case SYS_read: SYSCALL_ARG1(r) = sys_read(a[1], a[2], a[3]); break;
	case SYS_lseek: SYSCALL_ARG1(r) = sys_lseek(a[1], a[2], a[3]); break;
	case SYS_close: SYSCALL_ARG1(r) = sys_close(a[1]); break;
	
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
