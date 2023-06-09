#include "elf.h"

#include "compile.h"
#include "fatal_error.h"
#include "file.h"
#include "parser.h"
#include "str_table.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ELF_NULL_SECTION_INDEX     0
#define ELF_TEXT_SECTION_INDEX     1
#define ELF_DATA_SECTION_INDEX     2
#define ELF_BSS_SECTION_INDEX      3
#define ELF_SYMTAB_SECTION_INDEX   4
#define ELF_STRTAB_SECTION_INDEX   5
#define ELF_SHSTRTAB_SECTION_INDEX 6
#define ELF_NUM_SECTIONS           7

#define EV_NONE    0
#define EV_CURRENT 1

#define EM_RISCV 243

#define ET_NONE 0
#define ET_REL  1
#define ET_EXEC 2
#define ET_DYN  3
#define ET_CORE 4

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6

#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9
#define SHT_SHLIB   10
#define SHT_DYNSYM  11

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
    bool set_code_start;

    struct token* entry;

    uint64_t code_start;
    uint64_t code_size;

    uint64_t data_start;
    uint64_t data_size;

    uint64_t bss_start;
    uint64_t bss_size;

    struct str_table* function_table;
    struct str_table* object_table;

    struct executable_address_tuple** addresses;
    uint64_t addresses_length;

    struct elf_header* header;

    struct vector symtab;
    struct elf_symbol* text_symbol;
    struct elf_symbol* data_symbol;
    struct elf_symbol* bss_symbol;

    struct vector strtab;
    struct vector shstrtab;

    struct elf_program_header* code_program_header;
    struct elf_program_header* object_program_header;

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
    elf_file->set_code_start = false;
    elf_file->data_size = 0;
    elf_file->bss_size = 0;

    struct elf_header* elf_header
        = calloc(1, sizeof(struct elf_header));
    if (elf_header == NULL) {
        fatal_error("out of memory");
    }
    elf_file->header = elf_header;

    elf_file->function_table = str_table_create();
    elf_file->object_table = str_table_create();

    elf_header->magic[0] = 0x7F;
    elf_header->magic[1] = 'E';
    elf_header->magic[2] = 'L';
    elf_header->magic[3] = 'F';
    elf_header->bitness = 2;
    elf_header->endianness = 1;
    elf_header->version = EV_CURRENT;
    elf_header->os_abi = 0;
    elf_header->os_abi_version = 0;
    for (int i = 0; i < 7; ++i) {
        elf_header->padding[i] = 0;
    }
    elf_header->type = ET_EXEC;
    elf_header->machine = EM_RISCV;
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

    elf_file->code_program_header = elf_program_header_create_empty();

    elf_file->object_program_header = elf_program_header_create_empty();

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

    struct elf_symbol* data_symbol = symtab_next(symtab);
    data_symbol->name = strtab_add_from_c_str(strtab, ".data");
    data_symbol->info = ST_INFO(STB_LOCAL, STT_SECTION);
    data_symbol->other = ST_VISIBILITY(STV_DEFAULT);
    data_symbol->shndx = ELF_DATA_SECTION_INDEX;
    data_symbol->size = 0;
    elf_file->data_symbol = data_symbol;

    struct elf_symbol* bss_symbol = symtab_next(symtab);
    bss_symbol->name = strtab_add_from_c_str(strtab, ".bss");
    bss_symbol->info = ST_INFO(STB_LOCAL, STT_SECTION);
    bss_symbol->other = ST_VISIBILITY(STV_DEFAULT);
    bss_symbol->shndx = ELF_BSS_SECTION_INDEX;
    bss_symbol->size = 0;
    elf_file->bss_symbol = bss_symbol;

    struct elf_section_header* data_header
        = elf_section_header_get(elf_file, ELF_DATA_SECTION_INDEX);
    data_header->name = strtab_add_from_c_str(shstrtab, ".data");
    data_header->type = SHT_PROGBITS;
    data_header->flags = SHF_ALLOC | SHF_WRITE;
    data_header->link = 0;
    data_header->info = 0;
    data_header->addralign = 8;
    data_header->entsize = 0;

    struct elf_section_header* bss_header
        = elf_section_header_get(elf_file, ELF_BSS_SECTION_INDEX);
    bss_header->name = strtab_add_from_c_str(shstrtab, ".bss");
    bss_header->type = SHT_NOBITS;
    bss_header->flags = SHF_ALLOC | SHF_WRITE;
    bss_header->link = 0;
    bss_header->info = 0;
    bss_header->addralign = 8;
    bss_header->entsize = 0;

    struct elf_section_header* symtab_header
        = elf_section_header_get(elf_file, ELF_SYMTAB_SECTION_INDEX);
    symtab_header->name
        = strtab_add_from_c_str(shstrtab, ".symtab");
    symtab_header->type = SHT_SYMTAB;
    symtab_header->flags = 0;
    symtab_header->address = 0;
    symtab_header->link = ELF_STRTAB_SECTION_INDEX;
    /* symtab_header->info set elsewhere */
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

void elf_file_set_entry(struct elf_file* elf_file, struct token* name) {
    elf_file->entry = name;
    elf_file->set_entry = true;
}

void elf_file_set_code_start(struct elf_file* elf_file, uint64_t address) {
    if (elf_file->set_code_start) {
        fatal_error("code program header already set");
    }

    elf_file->code_start = address;

    elf_file->code_program_header->type = PT_LOAD;
    elf_file->code_program_header->flags = PF_R | PF_X;
    elf_file->code_program_header->virtual_address = address;
    elf_file->code_program_header->physical_address = address;
    elf_file->code_program_header->alignment = 0x1000;

    elf_file->set_code_start = true;
}

void elf_file_set_addresses(struct elf_file* elf_file,
                            struct executable_address_tuple** addresses,
                            uint64_t addresses_length) {
    elf_file->addresses = addresses;
    elf_file->addresses_length = addresses_length;
}

void elf_add_function(struct elf_file* elf_file,
                      struct function_ast_node* function_ast_node,
                      struct vector* instructions) {
    struct str* function_name = &(function_ast_node->name->str);

    struct elf_symbol* symbol = symtab_next(&elf_file->symtab);
    symbol->name
        = strtab_add_from_str(&elf_file->strtab, function_name);
    symbol->info = ST_INFO(STB_LOCAL, STT_FUNC);
    symbol->other = ST_VISIBILITY(STV_DEFAULT);
    symbol->shndx = ELF_TEXT_SECTION_INDEX;
    symbol->size = instructions->size;

    struct function_table_entry* entry
        = calloc(1, sizeof(struct function_table_entry));
    entry->function_ast_node = function_ast_node;
    entry->instructions = instructions;
    entry->symbol = symbol;
    entry->address = 0;
    str_table_insert(elf_file->function_table, function_name, entry);
}

static bool instructions_need_function_table(struct instructions_ast_node* insts) {
    for (uint64_t i = 0; i < insts->length; ++i) {
        struct ast_node* ast_node = insts->ast_nodes[i];
        if (is_ujtype_ast_node(ast_node)) {
            struct ujtype_ast_node* ujtype = (struct ujtype_ast_node*) ast_node;
            if (!ujtype->needs_function_table) {
                continue;
            }
            if (ujtype->offset_token->kind != TOKEN_IDENTIFIER) {
                fatal_error("expected offset to function nanme");
            }
            return true;
        }
    }
    return false;
}

void elf_add_uninitialized_data(
    struct elf_file* elf_file,
    struct uninitialized_data_ast_node* uninitialized_data_ast_node
) {
    uninitialized_data_ast_node->offset = elf_file->bss_size;
    elf_file->bss_size += uninitialized_data_ast_node->size;
    str_table_insert(elf_file->object_table,
                     &(uninitialized_data_ast_node->name->str),
                     uninitialized_data_ast_node);
}

void elf_file_finalize(struct elf_file* elf_file) {
    if (!elf_file->set_code_start) {
        fatal_error("elf file code start not set");
    }
    if (!elf_file->set_entry) {
        fatal_error("elf file entry address not set");
    }

    struct str_table_entry* function_entry = NULL;

    for (uint64_t i = 0; i < elf_file->addresses_length; ++i) {
        struct executable_address_tuple* tuple = elf_file->addresses[i];
        struct str* function_name = &(tuple->function->str);
        function_entry = str_table_get(elf_file->function_table, function_name);
        if (function_entry == NULL) {
            fatal_error("address set for unknown function");
        }
        uint64_t address = tuple->imm;

        struct function_table_entry* entry = function_entry->val;
        entry->address = address;
        entry->symbol->value = address;

        uint64_t code_end = address + entry->instructions->size;
        if (code_end <= elf_file->code_start) {
            fatal_error("end of code needs to come after start");
        }

        uint64_t needed_code_size = code_end - elf_file->code_start;
        if (needed_code_size > elf_file->code_size) {
            elf_file->code_size = needed_code_size;
        }
    }

    function_entry = str_table_iterator(elf_file->function_table);
    while (function_entry != NULL) {
        struct function_table_entry* entry = function_entry->val;

        /* TODO: just put the function at the end */
        if (entry->address == 0) {
            uint64_t address = elf_file->code_start + elf_file->code_size;
            entry->address = address;
            entry->symbol->value = address;

            elf_file->code_size += entry->instructions->size;
        }

        str_table_iterator_next(elf_file->function_table, &function_entry);
    }

    /* elf_file->code_size is finalized */

    uint64_t code_end = elf_file->code_start + elf_file->code_size;
    uint64_t data_start = code_end;
    if ((data_start % 0x1000) != 0) {
        data_start &= ~0xFFF;
        data_start += 0x1000;
    }
    elf_file->data_start = data_start;
    elf_file->bss_start = data_start + elf_file->data_size;

    /* Add all the objects to the symbol table */
    struct str_table_entry* object_entry
        = str_table_iterator(elf_file->object_table);
    while (object_entry != NULL) {
        struct ast_node* node = object_entry->val;

        if (is_uninitialized_data_ast_node(node)) {
            struct uninitialized_data_ast_node* uninitialized
                = (struct uninitialized_data_ast_node*) node;

            struct str* object_name = &(uninitialized->name->str);
            struct elf_symbol* symbol = symtab_next(&elf_file->symtab);
            symbol->name
                = strtab_add_from_str(&elf_file->strtab, object_name);
            symbol->info = ST_INFO(STB_LOCAL, STT_OBJECT);
            symbol->other = ST_VISIBILITY(STV_DEFAULT);
            symbol->shndx = ELF_BSS_SECTION_INDEX;
            symbol->value = elf_file->bss_start + uninitialized->offset;
            symbol->size = uninitialized->size;
        }

        str_table_iterator_next(elf_file->object_table, &object_entry);
    }

    elf_file->object_program_header->type = PT_LOAD;
    elf_file->object_program_header->flags = PF_R | PF_W;
    elf_file->object_program_header->virtual_address = elf_file->data_start;
    elf_file->object_program_header->physical_address = elf_file->data_start;
    elf_file->object_program_header->file_size = elf_file->data_size;
    elf_file->object_program_header->memory_size
        = elf_file->data_size + elf_file->bss_size;
    elf_file->object_program_header->alignment = 0x1000;

    {
        function_entry = str_table_get(elf_file->function_table,
                                       &elf_file->entry->str);
        if (function_entry == NULL) {
            fatal_error("entry function does not exist");
        }
        struct function_table_entry* entry = function_entry->val;
        if (entry->address == 0) {
            fatal_error("entry function address not set");
        }
        elf_file->header->entry = entry->address;
    }

    elf_file->code_program_header->file_size = elf_file->code_size;
    elf_file->code_program_header->memory_size = elf_file->code_size;

    elf_file->header->program_header_num_entries = 2;
    elf_file->header->section_header_num_entries = ELF_NUM_SECTIONS;
    elf_file->header->section_header_string_index = ELF_SHSTRTAB_SECTION_INDEX;

    /* .text section and symbol */
    struct elf_section_header* text_header
        = elf_section_header_get(elf_file, ELF_TEXT_SECTION_INDEX);
    text_header->address = elf_file->code_start;
    text_header->size = elf_file->code_size;

    elf_file->text_symbol->value = elf_file->code_start;

    /* .data section and symbol */
    struct elf_section_header* data_header
        = elf_section_header_get(elf_file, ELF_DATA_SECTION_INDEX);
    data_header->address = elf_file->data_start;
    data_header->size = elf_file->data_size;

    elf_file->data_symbol->value = elf_file->data_start;

    struct elf_section_header* bss_header
        = elf_section_header_get(elf_file, ELF_BSS_SECTION_INDEX);
    bss_header->address = elf_file->data_start;
    bss_header->size = elf_file->bss_size;

    elf_file->bss_symbol->value = elf_file->bss_start;

    struct elf_section_header* symtab_header
        = elf_section_header_get(elf_file, ELF_SYMTAB_SECTION_INDEX);
    symtab_header->size = elf_file->symtab.size;
    /* TODO: Better way to determine the first non-local symbol */
    /* Currently there's and entry for null, .text, .data, and .bss */
    symtab_header->info = 4 + str_table_size(elf_file->function_table)
                            + str_table_size(elf_file->object_table);

    struct elf_section_header* strtab_header
        = elf_section_header_get(elf_file, ELF_STRTAB_SECTION_INDEX);
    strtab_header->size = elf_file->strtab.size;

    struct elf_section_header* shstrtab_header
        = elf_section_header_get(elf_file, ELF_SHSTRTAB_SECTION_INDEX);
    shstrtab_header->size = elf_file->shstrtab.size;

    /* Compute all the offsets */
    uint64_t current_offset = sizeof(struct elf_header);
    elf_file->header->program_header_offset = current_offset;

    current_offset += sizeof(struct elf_program_header);
    current_offset += sizeof(struct elf_program_header);
    elf_file->code_program_header->offset = current_offset;
    text_header->offset = current_offset;

    current_offset += text_header->size;
    elf_file->object_program_header->offset = current_offset;
    data_header->offset = current_offset;

    current_offset += 0; /* TODO: Room for data */
    bss_header->offset = current_offset;
    symtab_header->offset = current_offset;

    current_offset += elf_file->symtab.size;
    strtab_header->offset = current_offset;

    current_offset += elf_file->strtab.size;
    shstrtab_header->offset = current_offset;

    current_offset += elf_file->shstrtab.size;
    elf_file->header->section_header_offset = current_offset;

    /* Final check for jumps to a label */
    function_entry = str_table_iterator(elf_file->function_table);
    while (function_entry != NULL) {
        struct function_table_entry* entry = function_entry->val;
        if (entry->address == 0) {
            fatal_error("function address not set");
        }

        struct function_ast_node* function_ast_node = entry->function_ast_node;
        if (instructions_need_function_table(function_ast_node->insts)) {
            instructions_recreate(entry, elf_file->function_table);
        }

        str_table_iterator_next(elf_file->function_table, &function_entry);
    }
}

void elf_write(struct elf_file* elf_file, const char* output_path) {
    int fd = file_open_write(output_path);

    ssize_t bytes_expected = sizeof(struct elf_header);
    ssize_t bytes_written = write(fd, elf_file->header, bytes_expected);
    if (bytes_written != bytes_expected) {
        fatal_error("write failed (header)");
    }

    bytes_expected = sizeof(struct elf_program_header);
    bytes_written = write(fd, elf_file->code_program_header, bytes_expected);
    if (bytes_written != bytes_expected) {
        fatal_error("write failed (code program header)");
    }

    bytes_expected = sizeof(struct elf_program_header);
    bytes_written = write(fd, elf_file->object_program_header, bytes_expected);
    if (bytes_written != bytes_expected) {
        fatal_error("write failed (data program header)");
    }

    /* Write all the code to the text section */
    off_t off = lseek(fd, 0, SEEK_CUR);
    if (off == -1) {
        fatal_error("lseek");
    }
    off_t code_start = off;
    struct str_table_entry* function_entry
        = str_table_iterator(elf_file->function_table);
    while (function_entry != NULL) {
        struct function_table_entry* entry = function_entry->val;
        if (entry->address == 0) {
            fatal_error("function address not set");
        }

        uint64_t code_offset = entry->address - elf_file->code_start;
        off = lseek(fd, code_start + code_offset, SEEK_SET);
        if (off == -1) {
            fatal_error("lseek");
        }

        bytes_expected = entry->instructions->size;
        bytes_written = write(fd, entry->instructions->data, bytes_expected);
        if (bytes_written != bytes_expected) {
            fatal_error("write failed (instructions)");
        }

        str_table_iterator_next(elf_file->function_table, &function_entry);
    }
    off = lseek(fd, code_start + elf_file->code_size, SEEK_SET);
    if (off == -1) {
        fatal_error("lseek");
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
