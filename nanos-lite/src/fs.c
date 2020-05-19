#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(void *buf, off_t offset, size_t len);

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

size_t fs_filesz(int fd) {
  if (fd<0 || fd>NR_FILES-1) {
    panic("File: %d not found", fd);
  }
  return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, mode_t mode) {
  for (int i=0; i<NR_FILES; i++) {
    if (strcmp(file_table[i].name, pathname)==0) {
	  file_table[i].open_offset = 0;
	  return i;
	}
  }
  // assert(0);
  panic("File: %s not found.\n", pathname);
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t count) {
  if (fd<0 || fd>NR_FILES-1) { 
	// assert(0); 
	panic("No such file.\n");
  }
  if (fd<3) { 
	// panic("fd: %d\n", fd);
	Log("fd: %d\n", fd);
  }

  off_t op_off = file_table[fd].open_offset;
  int remain = file_table[fd].size - op_off;
  int len = count;
  if (remain<count) { len = remain; }
  ramdisk_read(buf, file_table[fd].disk_offset + op_off, len);
  file_table[fd].open_offset = op_off + len;
  return len;
}

ssize_t fs_write(int fd, const void *buf, size_t count) {
  return count;
  return -1;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  return offset;
  return (off_t)-1;
}

int fs_close(int fd) {
  if (fd<0 || fd>NR_FILES-1) {
    panic("File: %d not found", fd);
  }
  return 0;
}
