#include "file.h"

#include "fatal_error.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct str file_open(const char* path) {
    struct str str = {
        .data = NULL,
        .size = 0,
    };

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        fatal_error("file open filed");
    }

    struct stat stat;
    if (fstat(fd, &stat) == -1) {
        fatal_error("file fstat failed");
    }

    str.data = mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (str.data == MAP_FAILED) {
        fatal_error("file mmap failed");
    }

    if (close(fd) == -1) {
        fatal_error("file close failed");
    }

    str.size = stat.st_size;

    return str;
}

void file_close(struct str* str) {
    if (munmap(str->data, str->size) == -1) {
        fatal_error("file munmap failed");
    }
}
