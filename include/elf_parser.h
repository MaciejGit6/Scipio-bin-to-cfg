#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------
// ELF64 Main Header Structure
// ---------------------------------------------------------
// This sits at the absolute beginning of the file (Offset 0)
typedef struct {
    unsigned char e_ident[16]; // Magic Bytes and architecture info (0x7f 'E' 'L' 'F')
    uint16_t      e_type;      // Object file type (e.g., Executable, Shared Object)
    uint16_t      e_machine;   // Target Architecture (e.g., x86-64, ARM)
    uint32_t      e_version;   // Object file version
    uint64_t      e_entry;     // Virtual memory address of the entry point
    uint64_t      e_phoff;     // Program header table file offset
    uint64_t      e_shoff;     // Section header table file offset (Important!)
    uint32_t      e_flags;     // Processor-specific flags
    uint16_t      e_ehsize;    // Size of this ELF header
    uint16_t      e_phentsize; // Size of a program header table entry
    uint16_t      e_phnum;     // Number of program header entries
    uint16_t      e_shentsize; // Size of a section header table entry
    uint16_t      e_shnum;     // Number of section header entries
    uint16_t      e_shstrndx;  // Index of the section header string table
} Elf64_Ehdr;


// ---------------------------------------------------------
// ELF64 Section Header Structure
// ---------------------------------------------------------
// An array of these sits at the end of the file, describing 
// where the .text, .data, and .rodata sections live.
typedef struct {
    uint32_t   sh_name;      // Index into the string table for the section name
    uint32_t   sh_type;      // Section type (e.g., PROGBITS, SYMTAB)
    uint64_t   sh_flags;     // Section attributes (e.g., Executable, Writable)
    uint64_t   sh_addr;      // Virtual address in memory where this section is loaded
    uint64_t   sh_offset;    // Offset in the physical file where the raw bytes start
    uint64_t   sh_size;      // Size of the section in bytes
    uint32_t   sh_link;      // Link to another section
    uint32_t   sh_info;      // Additional section information
    uint64_t   sh_addralign; // Address alignment boundaries
    uint64_t   sh_entsize;   // Size of entries, if the section contains a table
} Elf64_Shdr;


// ---------------------------------------------------------
// Function Prototypes
// ---------------------------------------------------------
// Parses the ELF file using mmap() and prints the section layout
int parse_elf_header(const char* filepath);
// Define a function pointer type for the C++ callback
typedef void (*InstructionCallback)(uint64_t address, const char* mnemonic, const char* operands);

// Update your disassembler prototype to accept this callback
int disassemble_text_section(const char* filepath, InstructionCallback callback);


#ifdef __cplusplus
}
#endif

#endif // ELF_PARSER_H