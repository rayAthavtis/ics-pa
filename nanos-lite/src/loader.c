#include "common.h"

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))
extern void ramdisk_read(void *buf, off_t offset, size_t len);
#define DEFAULT_ENTRY ((void *)0x8048000)

extern size_t fs_filesz(int fd);
extern int fs_open(const char *pathname, int flags, mode_t mode);
extern ssize_t fs_read(int fd, void *buf, size_t count);
extern ssize_t fs_close(int fd);

extern void _map(_Protect *p, void *va, void *pa);
extern void *new_page();

uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  // ramdisk_read(DEFAULT_ENTRY, 0, RAMDISK_SIZE);
 
  int fd = fs_open(filename, 0, 0);
  size_t size = fs_filesz(fd);
  Log("File: %s", filename);
  // fs_read(fd, DEFAULT_ENTRY, size);
  // fs_close(fd);

	void *pa, *va;
	void *max_size = DEFAULT_ENTRY + size;
	for (va = DEFAULT_ENTRY; va<max_size; va+=PGSIZE) {
		pa = new_page();
		_map(as, va, pa);
		fs_read(fd, pa, PGSIZE);
	}
	fs_close(fd);

  return (uintptr_t)DEFAULT_ENTRY;
}
