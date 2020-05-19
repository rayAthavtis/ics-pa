#ifndef __FS_H__
#define __FS_H__

#include "common.h"

enum {SEEK_SET, SEEK_CUR, SEEK_END};

size_t fs_filesz(int fd);
int fs_open(const char *pathname, int flags, mode_t mode);
ssize_t fs_read(int fd, void *buf, size_t count);
ssize_t fs_write(int fd, void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int fs_close(int fd);

#endif
