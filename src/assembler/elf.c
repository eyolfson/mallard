#include "elf.h"

#include "fatal_error.h"
#include "file.h"

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

struct elf_header {
    uint8_t magic[4];
    uint8_t bitness;
    uint8_t endianness;
    uint8_t version;
    uint8_t os_abi;
    uint8_t os_abi_version;
    uint8_t padding[7];

    uint16_t type;
    uint16_t machine;
    uint32_t elf_version;
    uint64_t entry;
    uint64_t program_header_offset;
    uint64_t section_header_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_header_entry_size;
    uint16_t program_header_num_entries;
    uint16_t section_header_entry_size;
    uint16_t section_header_num_entries;
    uint16_t section_header_string_index;
};

struct elf_program_header {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t virtual_address;
    uint64_t physical_address;
    uint64_t file_size;
    uint64_t memory_size;
    uint64_t alignment;
};

struct elf_section_header {
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t address;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t addralign;
    uint64_t entsize;
};

struct elf_symbol {
    uint32_t name;
    uint8_t info;
    uint8_t other;
    uint16_t shndx;
    uint64_t value;
    uint64_t size;
};

struct elf_file {
    bool set_entry;
    struct elf_header* header;
};

struct elf_file* elf_create_empty() {
    struct elf_file* elf_file = malloc(sizeof(struct elf_file));
    if (elf_file == NULL) {
        fatal_error("out of memory");
    }
    elf_file->set_entry = false;

    struct elf_header* elf_header
        = malloc(sizeof(struct elf_header));
    if (elf_header == NULL) {
        fatal_error("out of memory");
    }
    elf_file->header = elf_header;

    elf_header->magic[0] = 0x7F;
    elf_header->magic[1] = 'E';
    elf_header->magic[2] = 'L';
    elf_header->magic[3] = 'F';
    elf_header->bitness = 2;
    elf_header->endianness = 1;
    elf_header->version = 1;
    elf_header->os_abi = 0;
    elf_header->os_abi_version = 0;
    for (int i = 0; i < 7; ++i) {
        elf_header->padding[i] = 0;
    }
    elf_header->type = 2;
    elf_header->machine = 0xF3;
    elf_header->elf_version = 1;
    elf_header->entry = 0;
    elf_file->header->flags = 0;
    elf_file->header->header_size = sizeof(struct elf_header);
    elf_file->header->program_header_entry_size
        = sizeof(struct elf_program_header);
    elf_file->header->section_header_entry_size
        = sizeof(struct elf_section_header);

    return elf_file;
}

void elf_set_entry(struct elf_file* elf_file, uint32_t address) {
    elf_file->header->entry = address;
    elf_file->set_entry = true;
}

void elf_add_function(struct elf_file*,
                      struct token* name,
                      uint32_t address,
                      struct vector* instructions) {
    // Add symbol
    // Adjust program header
}

static void finalize_header(struct elf_file* elf_file) {
    elf_file->header->program_header_offset = 0;
    elf_file->header->section_header_offset = 0;
    elf_file->header->program_header_num_entries = 0;
    elf_file->header->section_header_num_entries = 0;
    elf_file->header->section_header_string_index = 0;
}

void elf_write(struct elf_file* elf_file, const char* output) {
    if (!elf_file->set_entry) {
        fatal_error("elf file has no entry address");
    }

    finalize_header(elf_file);

    int fd = file_open_write(output);

    ssize_t bytes_expected = sizeof(struct elf_header);
    ssize_t bytes_written = write(fd, elf_file->header, bytes_expected);
    if (bytes_written != bytes_expected) {
        fatal_error("write failed");
    }

    file_close(fd);
}
