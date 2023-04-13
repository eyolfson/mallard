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
    bool set_program_header;

    struct elf_header* header;

    struct vector strtab;
    struct vector shstrtab;

    struct elf_program_header* program_header;

    struct elf_section_header* null_section_header;
    struct elf_section_header* text_section_header;
    struct elf_section_header* symtab_section_header;
    struct elf_section_header* strtab_section_header;
    struct elf_section_header* shstrtab_section_header;
};

static struct elf_program_header* elf_program_header_create_empty() {
    struct elf_program_header* elf_program_header
        = calloc(1, sizeof(struct elf_program_header));
    if (elf_program_header == NULL) {
        fatal_error("out of memory");
    }
    return elf_program_header;
}

static struct elf_section_header* elf_section_header_create_empty() {
    struct elf_section_header* elf_section_header
        = calloc(1, sizeof(struct elf_section_header));
    if (elf_section_header == NULL) {
        fatal_error("out of memory");
    }
    return elf_section_header;
}

static struct vector strtab_create_empty() {
    uint64_t capacity = 4096;
    uint8_t* data = calloc(1, capacity);
    if (data == NULL) {
        fatal_error("out of memory");
    }
    uint64_t size = 1;
    struct vector strtab = {
         .capacity = capacity,
         .data = data,
         .size = size,
    };
    return strtab;
}

struct elf_file* elf_create_empty() {
    struct elf_file* elf_file = calloc(1, sizeof(struct elf_file));
    if (elf_file == NULL) {
        fatal_error("out of memory");
    }
    elf_file->set_entry = false;

    struct elf_header* elf_header
        = calloc(1, sizeof(struct elf_header));
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

    elf_file->strtab = strtab_create_empty();
    elf_file->shstrtab = strtab_create_empty();

    elf_file->program_header = elf_program_header_create_empty();

    elf_file->null_section_header = elf_section_header_create_empty();
    elf_file->text_section_header = elf_section_header_create_empty();
    elf_file->symtab_section_header = elf_section_header_create_empty();
    elf_file->strtab_section_header = elf_section_header_create_empty();
    elf_file->shstrtab_section_header = elf_section_header_create_empty();

    return elf_file;
}

void elf_set_entry(struct elf_file* elf_file, uint32_t address) {
    elf_file->header->entry = address;
    elf_file->set_entry = true;
}

static void elf_program_header_init(struct elf_file* elf_file,
                                    uint32_t address,
                                    struct vector* instructions) {
    if (elf_file->set_program_header) {
        fatal_error("program header already set");
    }

    elf_file->program_header->type = 1;
    elf_file->program_header->flags = 0x5;
    elf_file->program_header->virtual_address = address;
    elf_file->program_header->physical_address = address;
    elf_file->program_header->file_size = instructions->size;
    elf_file->program_header->memory_size = instructions->size;
    elf_file->program_header->alignment = 0;

    elf_file->set_program_header = true;
}

void elf_add_function(struct elf_file* elf_file,
                      struct token* name,
                      uint32_t address,
                      struct vector* instructions) {
    elf_program_header_init(elf_file, address, instructions);
    /* TODO: add symbol to symbol table */
}

static void finalize_header(struct elf_file* elf_file) {
    elf_file->header->program_header_num_entries = 1;
    elf_file->header->section_header_num_entries = 5;
    elf_file->header->section_header_string_index = 4;

    elf_file->header->program_header_offset = 0;
    elf_file->header->section_header_offset = 0;

    elf_file->program_header->offset = 0 /* TODO: where the instructions are */;
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
