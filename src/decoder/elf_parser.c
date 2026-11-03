#include "elf_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      
#include <sys/mman.h>   
#include <sys/stat.h>   
#include <unistd.h>     
#include <capstone/capstone.h>

#define ELFCLASS64   2
#define ELFDATA2LSB  1
#define EM_X86_64    62
#define SHT_NOBITS   8

typedef struct {
    uint8_t*            data;
    size_t              size;
    Elf64_Ehdr*         hdr;
    Elf64_Shdr*         sections;   // NULL if the file has no section headers
    const Elf64_Shdr*   shstrtab;
} MappedElf;

// True if [offset, offset + len) lies inside the file (overflow-safe)
static int in_bounds(const MappedElf* elf, uint64_t offset, uint64_t len) {
    return offset <= elf->size && len <= elf->size - offset;
}

static int section_in_bounds(const MappedElf* elf, const Elf64_Shdr* shdr) {
    if (shdr->sh_type == SHT_NOBITS) return 1;
    return in_bounds(elf, shdr->sh_offset, shdr->sh_size);
}

// Returns the section name, or "" if it can't be resolved safely
static const char* section_name(const MappedElf* elf, const Elf64_Shdr* shdr) {
    if (!elf->shstrtab || shdr->sh_name >= elf->shstrtab->sh_size) return "";
    const char* name = (const char*)(elf->data + elf->shstrtab->sh_offset + shdr->sh_name);
    size_t max_len = elf->shstrtab->sh_size - shdr->sh_name;
    return memchr(name, '\0', max_len) ? name : "";
}

static void unmap_elf(MappedElf* elf) {
    if (elf->data) munmap(elf->data, elf->size);
    elf->data = NULL;
}

// Maps the file and validates that it is a well-formed 64-bit little-endian
// x86-64 ELF. Returns 0 on success, -1 (after printing why) on failure.
static int map_elf(const char* filepath, MappedElf* elf) {
    memset(elf, 0, sizeof(*elf));

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        perror("[-] Error: Could not open file");
        return -1;
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0) {
        perror("[-] Error: Could not get file stats");
        close(fd);
        return -1;
    }

    if ((size_t)file_stat.st_size < sizeof(Elf64_Ehdr)) {
        printf("[-] Error: File too small to be an ELF binary.\n");
        close(fd);
        return -1;
    }

    void* mapped = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd); // the mapping stays valid after close
    if (mapped == MAP_FAILED) {
        perror("[-] Error: mmap failed");
        return -1;
    }

    elf->data = (uint8_t*)mapped;
    elf->size = file_stat.st_size;
    elf->hdr  = (Elf64_Ehdr*)elf->data;

    const unsigned char* id = elf->hdr->e_ident;
    if (id[0] != 0x7f || id[1] != 'E' || id[2] != 'L' || id[3] != 'F') {
        printf("[-] Error: Not a valid ELF file.\n");
        unmap_elf(elf);
        return -1;
    }

    if (id[4] != ELFCLASS64 || id[5] != ELFDATA2LSB || elf->hdr->e_machine != EM_X86_64) {
        printf("[-] Error: Only 64-bit little-endian x86-64 ELF binaries are supported.\n");
        unmap_elf(elf);
        return -1;
    }

    if (elf->hdr->e_shnum > 0) {
        if (elf->hdr->e_shentsize != sizeof(Elf64_Shdr) ||
            !in_bounds(elf, elf->hdr->e_shoff,
                       (uint64_t)elf->hdr->e_shnum * sizeof(Elf64_Shdr))) {
            printf("[-] Error: Section header table is outside the file.\n");
            unmap_elf(elf);
            return -1;
        }
        elf->sections = (Elf64_Shdr*)(elf->data + elf->hdr->e_shoff);

        if (elf->hdr->e_shstrndx < elf->hdr->e_shnum &&
            section_in_bounds(elf, &elf->sections[elf->hdr->e_shstrndx]))
            elf->shstrtab = &elf->sections[elf->hdr->e_shstrndx];
    }

    return 0;
}

int parse_elf_header(const char* filepath) {
    MappedElf elf;
    if (map_elf(filepath, &elf) != 0)
        return -1;

    printf("[+] Verified: Valid ELF Header found.\n");
    printf("[+] Entry Point: 0x%lx\n", elf.hdr->e_entry);

    // Parsing Section Headers
    if (elf.sections) {
        printf("\n[*] Parsing %d Section Headers via mmap()...\n", elf.hdr->e_shnum);
        printf("%-20s %-15s %-10s\n", "NAME", "FILE OFFSET", "SIZE (Bytes)");
        printf("----------------------------------------------------\n");

        for (int i = 0; i < elf.hdr->e_shnum; i++) {
            Elf64_Shdr* shdr = &elf.sections[i];

            //Only printing sections that actually contain data
            if (shdr->sh_size > 0) {
                printf("%-20s 0x%-13lx %-10lu\n", section_name(&elf, shdr),
                       shdr->sh_offset, shdr->sh_size);
            }
        }
        printf("----------------------------------------------------\n");
    }

    unmap_elf(&elf);
    return 0;
}

// Reports every sized STT_FUNC symbol from .symtab. Returns the number reported
// (0 for a stripped binary).
static size_t report_functions(const MappedElf* elf, FunctionCallback callback) {
    for (int i = 0; i < elf->hdr->e_shnum; i++) {
        const Elf64_Shdr* shdr = &elf->sections[i];
        if (shdr->sh_type != SHT_SYMTAB || shdr->sh_entsize != sizeof(Elf64_Sym))
            continue;
        if (!section_in_bounds(elf, shdr) || shdr->sh_link >= elf->hdr->e_shnum)
            continue;

        const Elf64_Shdr* str_shdr = &elf->sections[shdr->sh_link];
        if (!section_in_bounds(elf, str_shdr))
            continue;

        const Elf64_Sym* symbols = (const Elf64_Sym*)(elf->data + shdr->sh_offset);
        size_t      sym_count = shdr->sh_size / shdr->sh_entsize;
        const char* sym_str   = (const char*)(elf->data + str_shdr->sh_offset);

        size_t n = 0;
        for (size_t s = 0; s < sym_count; s++) {
            const Elf64_Sym* sym = &symbols[s];
            if (ELF_ST_TYPE(sym->st_info) != STT_FUNC || sym->st_size == 0)
                continue;

            const char* name = "";
            if (sym->st_name < str_shdr->sh_size &&
                memchr(sym_str + sym->st_name, '\0', str_shdr->sh_size - sym->st_name))
                name = sym_str + sym->st_name;

            if (callback != NULL)
                callback(sym->st_value, sym->st_size, name);
            n++;
        }
        return n;
    }
    return 0; //stripped binary
}

int disassemble_text_section(const char* filepath,
                             InstructionCallback callback,
                             FunctionCallback func_callback) {
    MappedElf elf;
    if (map_elf(filepath, &elf) != 0)
        return -1;

    for (int i = 0; elf.sections && i < elf.hdr->e_shnum; i++) {
        Elf64_Shdr* shdr = &elf.sections[i];

        if (strcmp(section_name(&elf, shdr), ".text") != 0)
            continue;

        if (shdr->sh_type == SHT_NOBITS || !section_in_bounds(&elf, shdr)) {
            printf("[-] Error: .text section is outside the file.\n");
            unmap_elf(&elf);
            return -1;
        }

        uint8_t* text_code = elf.data + shdr->sh_offset;

        size_t func_count = report_functions(&elf, func_callback);
        printf("[*] Found %zu function symbols.\n", func_count);

        printf("\n[*] Initializing Capstone Engine (x86_64)...\n");

        csh handle;
        // Opening Capstone for x86 architecture, 64-bit mode
        if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
            printf("[-] Error: Failed to initialize Capstone!\n");
            unmap_elf(&elf);
            return -1;
        }

        // Without SKIPDATA, Capstone stops at the first undecodable byte and
        // silently drops the rest of .text
        cs_option(handle, CS_OPT_SKIPDATA, CS_OPT_ON);

        cs_insn *insn;
        // passing the virtual address (shdr->sh_addr) so the jump targets are calculated correctly!
        size_t count = cs_disasm(handle, text_code, shdr->sh_size, shdr->sh_addr, 0, &insn);

        int rc = 0;
        if (count > 0) {
            printf("[+] Disassembly successful. Sending instructions to C++ Graph Engine...\n");

            for (size_t j = 0; j < count; j++) {
                // Skipped data bytes are reported with id 0; they are not code
                if (insn[j].id == 0) continue;

                if (callback != NULL) {
                    callback(insn[j].address, insn[j].mnemonic, insn[j].op_str);
                }
            }

            cs_free(insn, count);
        } else {
            printf("[-] Error: Failed to disassemble given code!\n");
            rc = -1;
        }

        cs_close(&handle);
        unmap_elf(&elf);
        return rc;
    }

    printf("[-] Error: No .text section found.\n");
    unmap_elf(&elf);
    return -1;
}

uint64_t get_elf_entry(const char* filepath) {
    MappedElf elf;
    if (map_elf(filepath, &elf) != 0)
        return 0;

    uint64_t entry = elf.hdr->e_entry;
    unmap_elf(&elf);
    return entry;
}
