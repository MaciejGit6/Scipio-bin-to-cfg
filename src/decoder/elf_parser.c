#include "elf_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      
#include <sys/mman.h>   
#include <sys/stat.h>   
#include <unistd.h>     

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

    //Cast the very start of the mapped memory to our ELF header struct
    Elf64_Ehdr* elf_hdr = (Elf64_Ehdr*)mapped_data;

    // Verify Magic Bytes
    if (elf_hdr->e_ident[0] != 0x7f || elf_hdr->e_ident[1] != 'E' ||
        elf_hdr->e_ident[2] != 'L'  || elf_hdr->e_ident[3] != 'F') {
        printf("[-] Error: Not a valid ELF file.\n");
        munmap(mapped_data, file_stat.st_size);
        close(fd);
        return -1;
    }

    printf("[+] Verified: Valid ELF Header found.\n");
    printf("[+] Entry Point: 0x%lx\n", elf_hdr->e_entry);

    // Parse Section Headers
    if (elf_hdr->e_shnum > 0) {
        printf("\n[*] Parsing %d Section Headers via mmap()...\n", elf_hdr->e_shnum);
        printf("%-20s %-15s %-10s\n", "NAME", "FILE OFFSET", "SIZE (Bytes)");
        printf("----------------------------------------------------\n");

        //Pointer arithmetic to find the start of the Section Header Table
        Elf64_Shdr* section_headers = (Elf64_Shdr*)(mapped_data + elf_hdr->e_shoff);

        // Find the String Table to get the section names
        Elf64_Shdr* strtab_hdr = &section_headers[elf_hdr->e_shstrndx];
        const char* string_table = (const char*)(mapped_data + strtab_hdr->sh_offset);

        // Loop through the array of section headers
        for (int i = 0; i < elf_hdr->e_shnum; i++) {
            Elf64_Shdr* shdr = &section_headers[i];
            const char* name = string_table + shdr->sh_name;

            //Only print sections that actually contain data
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