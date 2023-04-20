#include "elf.h"

#include "fatal_error.h"
#include "file.h"
#include "parser.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ELF_NULL_SECTION_INDEX     0
#define ELF_TEXT_SECTION_INDEX     1
#define ELF_SYMTAB_SECTION_INDEX   2
#define ELF_STRTAB_SECTION_INDEX   3
#define ELF_SHSTRTAB_SECTION_INDEX 4
#define ELF_NUM_SECTIONS           5

#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3

#define SHF_WRITE            0x1
#define SHF_ALLOC            0x2
#define SHF_EXECINSTR        0x4
#define SHF_MERGE            0x10
#define SHF_STRINGS          0x20
#define SHF_INFO_LINK        0x40
#define SHF_LINK_ORDER       0x80
#define SHF_OS_NONCONFORMING 0x100
#define SHF_GROUP            0x200
#define SHF_TLS              0x400
#define SHF_MASKOS           0x0FF00000
#define SHF_AMD64_LARGE      0x10000000
#define SHF_ORDERED          0x40000000
#define SHF_EXCLUDE          0x80000000
#define SHF_MASKPROC         0xF0000000

#define ST_BIND(info)       ((info) >> 4)
#define ST_TYPE(info)       ((info) & 0xf)
#define ST_INFO(bind, type) (((bind)<<4)+((type)&0xf))

#define STB_LOCAL   0
#define STB_GLOBAL  1
#define STB_WEAK    2
#define STB_LOOS   10
#define STB_HIOS   12
#define STB_LOPROC 13
#define STB_HIPROC 15

#define SHN_UNDEF 0

#define STT_NOTYPE          0
#define STT_OBJECT          1
#define STT_FUNC            2
#define STT_SECTION         3
#define STT_FILE            4
#define STT_COMMON          5
#define STT_TLS             6
#define STT_LOOS           10
#define STT_HIOS           12
#define STT_LOPROC         13
#define STT_SPARC_REGISTER 13
#define STT_HIPROC         15

#define ST_VISIBILITY(o) ((o)&0x3)

#define STV_DEFAULT   0
#define STV_INTERNAL  1
#define STV_HIDDEN    2
#define STV_PROTECTED 3
#define STV_EXPORTED  4
#define STV_SINGLETON 5
#define STV_ELIMINATE 6

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

    struct vector* instructions;

    struct vector symtab;
    struct elf_symbol* text_symbol;

    struct vector strtab;
    struct vector shstrtab;

    struct elf_program_header* program_header;

    struct vector section_headers;
};

static struct elf_program_header* elf_program_header_create_empty() {
    struct elf_program_header* elf_program_header
        = calloc(1, sizeof(struct elf_program_header));
    if (elf_program_header == NULL) {
        fatal_error("out of memory");
    }
    return elf_program_header;
}

static struct elf_section_header* elf_section_header_get(
    struct elf_file* elf_file,
    uint64_t index) {
    struct elf_section_header* section_headers
        = (struct elf_section_header*) elf_file->section_headers.data;
    return &section_headers[index];
}

static void symtab_create_empty(struct vector* symtab) {
    uint64_t capacity = 4096;
    uint8_t* data = calloc(1, capacity);
    if (data == NULL) {
        fatal_error("out of memory");
    }
    uint64_t size = 0;
    symtab->capacity = capacity;
    symtab->data = data;
    symtab->size = size;
}

static struct elf_symbol* symtab_next(struct vector* symtab) {
    size_t len = sizeof(struct elf_symbol);
    if ((symtab->size + len) > symtab->capacity) {
        fatal_error("symbols out of space");
    }
    uint32_t index = symtab->size / len;
    struct elf_symbol* data = (struct elf_symbol*) symtab->data;
    symtab->size += len;
    return &data[index];
}

static void strtab_create_empty(struct vector* strtab) {
    uint64_t capacity = 4096;
    uint8_t* data = calloc(1, capacity);
    if (data == NULL) {
        fatal_error("out of memory");
    }
    /* The size is 0 because we expect index 0 to always be null */
    uint64_t size = 1;
    strtab->capacity = capacity;
    strtab->data = data;
    strtab->size = size;
}

static uint32_t strtab_add_from_c_str(struct vector* strtab, const char* str) {
    size_t len = strlen(str) + 1;
    if ((strtab->size + len) > strtab->capacity) {
        fatal_error("strtab out of space");
    }
    uint32_t next_index = strtab->size;
    memcpy(&strtab->data[next_index], str, len);
    strtab->size += len;
    return next_index;
}

static uint32_t strtab_add_from_str(struct vector* strtab, struct str* str) {
    size_t len = str->size + 1;
    if ((strtab->size + len) > strtab->capacity) {
        fatal_error("strtab out of space");
    }
    uint32_t next_index = strtab->size;
    memcpy(&strtab->data[next_index], str->data, str->size);
    strtab->size += len;
    return next_index;
}

static void elf_section_headers_init(struct vector* section_headers) {
    uint64_t capacity = sizeof(struct elf_section_header) * ELF_NUM_SECTIONS;
    struct elf_section_header* data = calloc(1, capacity);
    if (data == NULL) {
        fatal_error("out of memory");
    }
    uint64_t size = capacity;

    section_headers->capacity = capacity;
    section_headers->data = (uint8_t*) data;
    section_headers->size = size;
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

    struct vector* symtab = &elf_file->symtab;
    symtab_create_empty(symtab);

    struct vector* strtab = &elf_file->strtab;
    strtab_create_empty(strtab);

    struct vector* shstrtab = &elf_file->shstrtab;
    strtab_create_empty(shstrtab);

    elf_file->program_header = elf_program_header_create_empty();

    elf_section_headers_init(&elf_file->section_headers);

    struct elf_symbol* null_symbol = symtab_next(symtab);
    null_symbol->name = 0;
    null_symbol->info = ST_INFO(STB_LOCAL, STT_NOTYPE);
    null_symbol->other = ST_VISIBILITY(STV_DEFAULT);
    null_symbol->shndx = SHN_UNDEF;
    null_symbol->size = 0;

    struct elf_symbol* text_symbol = symtab_next(symtab);
    text_symbol->name = strtab_add_from_c_str(strtab, ".text");
    text_symbol->info = ST_INFO(STB_LOCAL, STT_SECTION);
    text_symbol->other = ST_VISIBILITY(STV_DEFAULT);
    text_symbol->shndx = ELF_TEXT_SECTION_INDEX;
    text_symbol->size = 0;
    elf_file->text_symbol = text_symbol;

    struct elf_section_header* text_header
        = elf_section_header_get(elf_file, ELF_TEXT_SECTION_INDEX);
    text_header->name = strtab_add_from_c_str(shstrtab, ".text");
    text_header->type = SHT_PROGBITS;
    text_header->flags = SHF_ALLOC | SHF_EXECINSTR;
    text_header->link = 0;
    text_header->info = 0;
    text_header->addralign = 2;
    text_header->entsize = 0;

    struct elf_section_header* symtab_header
        = elf_section_header_get(elf_file, ELF_SYMTAB_SECTION_INDEX);
    symtab_header->name
        = strtab_add_from_c_str(shstrtab, ".symtab");
    symtab_header->type = SHT_SYMTAB;
    symtab_header->flags = 0;
    symtab_header->address = 0;
    symtab_header->link = ELF_STRTAB_SECTION_INDEX;
    /* TODO: One greater than the symbol table index of the last local symbol
             (binding STB_LOCAL). */
    symtab_header->info = 2; 
    symtab_header->addralign = 8;
    symtab_header->entsize = sizeof(struct elf_symbol);

    struct elf_section_header* strtab_header
        = elf_section_header_get(elf_file, ELF_STRTAB_SECTION_INDEX);
    strtab_header->name
        = strtab_add_from_c_str(shstrtab, ".strtab");
    strtab_header->type = SHT_STRTAB;
    strtab_header->flags = 0;
    strtab_header->address = 0;
    strtab_header->link = 0;
    strtab_header->info = 0;
    strtab_header->addralign = 1;
    strtab_header->entsize = 0;

    struct elf_section_header* shstrtab_header
        = elf_section_header_get(elf_file, ELF_SHSTRTAB_SECTION_INDEX);
    shstrtab_header->name
        = strtab_add_from_c_str(shstrtab, ".shstrtab");
    shstrtab_header->type = SHT_STRTAB;
    shstrtab_header->flags = 0;
    shstrtab_header->address = 0;
    shstrtab_header->link = 0;
    shstrtab_header->info = 0;
    shstrtab_header->addralign = 1;
    shstrtab_header->entsize = 0;

    return elf_file;
}

void elf_set_entry(struct elf_file* elf_file, uint32_t address) {
    elf_file->header->entry = address;

    elf_file->text_symbol->value = address;

    struct elf_section_header* text_header
        = elf_section_header_get(elf_file, ELF_TEXT_SECTION_INDEX);
    text_header->address = address;

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
    elf_file->instructions = instructions;
    elf_program_header_init(elf_file, address, instructions);

    struct elf_symbol* symbol = symtab_next(&elf_file->symtab);
    symbol->name
        = strtab_add_from_str(&elf_file->strtab, &name->str);
    symbol->info = ST_INFO(STB_GLOBAL, STT_FUNC);
    symbol->other = ST_VISIBILITY(STV_DEFAULT);
    symbol->shndx = ELF_TEXT_SECTION_INDEX;
    symbol->value = address;
    symbol->size = instructions->size;
}

static void elf_finalize(struct elf_file* elf_file) {
    if (!elf_file->set_entry) {
        fatal_error("entry address not set");
    }
    if (!elf_file->set_program_header) {
        fatal_error("program header not set");
    }

    elf_file->header->program_header_num_entries = 1;
    elf_file->header->section_header_num_entries = ELF_NUM_SECTIONS;
    elf_file->header->section_header_string_index = ELF_SHSTRTAB_SECTION_INDEX;

    struct elf_section_header* text_header
        = elf_section_header_get(elf_file, ELF_TEXT_SECTION_INDEX);
    text_header->size = elf_file->instructions->size;

    struct elf_section_header* symtab_header
        = elf_section_header_get(elf_file, ELF_SYMTAB_SECTION_INDEX);
    symtab_header->size = elf_file->symtab.size;

    struct elf_section_header* strtab_header
        = elf_section_header_get(elf_file, ELF_STRTAB_SECTION_INDEX);
    strtab_header->size = elf_file->strtab.size;

    struct elf_section_header* shstrtab_header
        = elf_section_header_get(elf_file, ELF_SHSTRTAB_SECTION_INDEX);
    shstrtab_header->size = elf_file->shstrtab.size;

    uint64_t current_offset = sizeof(struct elf_header);
    elf_file->header->program_header_offset = current_offset;

    current_offset += sizeof(struct elf_program_header);
    elf_file->program_header->offset = current_offset;
    text_header->offset = current_offset;

    current_offset += elf_file->instructions->size;
    symtab_header->offset = current_offset;

    current_offset += elf_file->symtab.size;
    strtab_header->offset = current_offset;

    current_offset += elf_file->strtab.size;
    shstrtab_header->offset = current_offset;

    current_offset += elf_file->shstrtab.size;
    elf_file->header->section_header_offset = current_offset;
}

void elf_write(struct elf_file* elf_file, const char* output_path) {
    if (!elf_file->set_entry) {
        fatal_error("elf file has no entry address");
    }

    elf_finalize(elf_file);

    int fd = file_open_write(output_path);

    ssize_t bytes_expected = sizeof(struct elf_header);
    ssize_t bytes_written = write(fd, elf_file->header, bytes_expected);
    if (bytes_written != bytes_expected) {
        fatal_error("write failed (header)");
    }

    bytes_expected = sizeof(struct elf_program_header);
    bytes_written = write(fd, elf_file->program_header, bytes_expected);
    if (bytes_written != bytes_expected) {
        fatal_error("write failed (program header)");
    }

    bytes_expected = elf_file->instructions->size;
    bytes_written = write(fd, elf_file->instructions->data, bytes_expected);
    if (bytes_written != bytes_expected) {
        fatal_error("write failed (instructions)");
    }

    bytes_expected = elf_file->symtab.size;
    bytes_written = write(fd, elf_file->symtab.data, bytes_expected);
    if (bytes_written != bytes_expected) {
        fatal_error("write failed (symtab)");
    }

    bytes_expected = elf_file->strtab.size;
    bytes_written = write(fd, elf_file->strtab.data, bytes_expected);
    if (bytes_written != bytes_expected) {
        fatal_error("write failed (strtab)");
    }

    bytes_expected = elf_file->shstrtab.size;
    bytes_written = write(fd, elf_file->shstrtab.data, bytes_expected);
    if (bytes_written != bytes_expected) {
        fatal_error("write failed (shstrtab)");
    }

    bytes_expected = elf_file->section_headers.size;
    bytes_written = write(fd, elf_file->section_headers.data, bytes_expected);
    if (bytes_written != bytes_expected) {
        fatal_error("write failed (section headers)");
    }

    file_close(fd);
}
