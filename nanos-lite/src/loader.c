#include "common.h"

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))
extern void ramdisk_read(void *buf, off_t offset, size_t len);
#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  ramdisk_read(DEFAULT_ENTRY, 0, RAMDISK_SIZE);
  return (uintptr_t)DEFAULT_ENTRY;
}
