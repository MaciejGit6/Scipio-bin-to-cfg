#include "elf_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      
#include <sys/mman.h>   
#include <sys/stat.h>   
#include <unistd.h>     
#include <capstone/capstone.h>

int parse_elf_header(const char* filepath) {

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

    uint8_t* mapped_data = (uint8_t*)mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_data == MAP_FAILED) {
        perror("[-] Error: mmap failed");
        close(fd);
        return -1;
    }

    //Casting the very start of the mapped memory to our ELF header struct
    Elf64_Ehdr* elf_hdr = (Elf64_Ehdr*)mapped_data;

    // Verifying Magic Bytes
    if (elf_hdr->e_ident[0] != 0x7f || elf_hdr->e_ident[1] != 'E' ||
        elf_hdr->e_ident[2] != 'L'  || elf_hdr->e_ident[3] != 'F') {
        printf("[-] Error: Not a valid ELF file.\n");
        munmap(mapped_data, file_stat.st_size);
        close(fd);
        return -1;
    }

    printf("[+] Verified: Valid ELF Header found.\n");
    printf("[+] Entry Point: 0x%lx\n", elf_hdr->e_entry);

    // Parsing Section Headers
    if (elf_hdr->e_shnum > 0) {
        printf("\n[*] Parsing %d Section Headers via mmap()...\n", elf_hdr->e_shnum);
        printf("%-20s %-15s %-10s\n", "NAME", "FILE OFFSET", "SIZE (Bytes)");
        printf("----------------------------------------------------\n");

        //Pointer arithmetic to find the start of the Section Header Table
        Elf64_Shdr* section_headers = (Elf64_Shdr*)(mapped_data + elf_hdr->e_shoff);

        // Finding the String Table to get the section names
        Elf64_Shdr* strtab_hdr = &section_headers[elf_hdr->e_shstrndx];
        const char* string_table = (const char*)(mapped_data + strtab_hdr->sh_offset);

        // Looping through the array of section headers
        for (int i = 0; i < elf_hdr->e_shnum; i++) {
            Elf64_Shdr* shdr = &section_headers[i];
            const char* name = string_table + shdr->sh_name;

            //Only printing sections that actually contain data
            if (shdr->sh_size > 0) {
                printf("%-20s 0x%-13lx %-10lu\n", name, shdr->sh_offset, shdr->sh_size);
            }
        }
        printf("----------------------------------------------------\n");
    }

    munmap(mapped_data, file_stat.st_size);
    close(fd);
    
    return 0;
}

int disassemble_text_section(const char* filepath, InstructionCallback callback) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return -1;

    struct stat file_stat;
    fstat(fd, &file_stat);
    uint8_t* mapped_data = (uint8_t*)mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    Elf64_Ehdr* elf_hdr = (Elf64_Ehdr*)mapped_data;
    Elf64_Shdr* section_headers = (Elf64_Shdr*)(mapped_data + elf_hdr->e_shoff);
    Elf64_Shdr* strtab_hdr = &section_headers[elf_hdr->e_shstrndx];
    const char* string_table = (const char*)(mapped_data + strtab_hdr->sh_offset);

    for (int i = 0; i < elf_hdr->e_shnum; i++) {
        Elf64_Shdr* shdr = &section_headers[i];
        const char* name = string_table + shdr->sh_name;

        if (strcmp(name, ".text") == 0) {
            uint8_t* text_code = mapped_data + shdr->sh_offset;
            
            printf("\n[*] Initializing Capstone Engine (x86_64)...\n");
            
            csh handle;
            // Opening Capstone for x86 architecture, 64-bit mode
            if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
                printf("[-] Error: Failed to initialize Capstone!\n");
                return -1;
            }

            cs_insn *insn;
            // Disassembling the first 64 bytes (to test)
            // passing the virtual address (shdr->sh_addr) so the jump targets are calculated correctly!
            size_t count = cs_disasm(handle, text_code, shdr->sh_size, shdr->sh_addr, 0, &insn);
            
            if (count > 0) {
                printf("[+] Disassembly successful. Sending instructions to C++ Graph Engine...\n");
                
                for (size_t j = 0; j < count; j++) {
                    // Instead of printing, we call the C++ function!
                    if (callback != NULL) {
                        callback(insn[j].address, insn[j].mnemonic, insn[j].op_str);
                    }
                }
                
                cs_free(insn, count);
            }else {
                printf("[-] Error: Failed to disassemble given code!\n");
            }

            cs_close(&handle);
            munmap(mapped_data, file_stat.st_size);
            close(fd);
            return 0;
        }
    }

    munmap(mapped_data, file_stat.st_size);
    close(fd);
    return -1;
}