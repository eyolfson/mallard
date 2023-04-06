#ifndef MALLARD_FILE_H
#define MALLARD_FILE_H

#include "str.h"

struct str file_open(const char* path);
void file_close(struct str* str);

#endif /* ifndef MALLARD_FILE_H */
