#include "file.h"

#include "fatal_error.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct str file_open_read_mmap(const char* path) {
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

void file_close_mmap(struct str* str) {
    if (munmap(str->data, str->size) == -1) {
        fatal_error("file munmap failed");
    }
}

int file_open_write(const char* path) {
    int fd = open(path, O_CREAT | O_WRONLY, 0755);
    if (fd == -1) {
        fatal_error("file open filed");
    }
    ftruncate(fd, 0);
    return fd;
}

void file_close(int fd) {
    if (close(fd) == -1) {
        fatal_error("file close failed");
    }
}
