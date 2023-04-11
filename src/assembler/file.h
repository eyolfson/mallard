#ifndef MALLARD_FILE_H
#define MALLARD_FILE_H

#include "str.h"

struct str file_open_read_mmap(const char* path);
void file_close_mmap(struct str* str);

int file_open_write(const char* path);
void file_close(int fd);

#endif /* ifndef MALLARD_FILE_H */
