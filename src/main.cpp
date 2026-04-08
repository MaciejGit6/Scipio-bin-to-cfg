
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>  // Required for readlink()
#include <limits.h>  // Required for PATH_MAX
#include "elf_parser.h"




void print_banner() {
    std::cout << R"(
   _____  _____ _____ _____ _____  ____  
  / ____|/ ____|_   _|  __ \_   _|/ __ \ 
 | (___ | |      | | | |__) || | | |  | |
  \___ \| |      | | |  ___/ | | | |  | |
  ____) | |____ _| |_| |    _| |_| |__| |
 |_____/ \_____|_____|_|   |_____|\____/ 
                                         
    )" << '\n';
    std::cout << "[*] Scipio Binary Analysis Tool - v0.1\n";
    std::cout << "[*] Initializing decoder engine...\n\n";
}

void print_readme() {
    char path_buffer[PATH_MAX];
    
    //  Read the absolute path of the currently running executable
    ssize_t count = readlink("/proc/self/exe", path_buffer, PATH_MAX);
    
    if (count != -1) {
        std::string exe_path(path_buffer, count); // e.g., "/home/milewskim/Scipio/scipio"
        
        // Find the last slash to isolate the directory path
        size_t last_slash_idx = exe_path.find_last_of('/');
        if (last_slash_idx != std::string::npos) {
            std::string dir_path = exe_path.substr(0, last_slash_idx);
            
            // Construct the absolute path to the README
            std::string readme_path = dir_path + "/README.md";
            
            // Open the file using the absolute path
            std::ifstream file(readme_path);
            if (file.is_open()) {
                std::string line;
                std::cout << "\n--- PROJECT DOCUMENTATION (README.md) ---\n\n";
                while (getline(file, line)) {
                    std::cout << line << std::endl;
                }
                std::cout << "\n------------------------------------------\n";
                file.close();
                return; // 
            }
        }
    }
    
    std::cout << "[-] Error: README.md not found alongside the executable.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2 || std::string(argv[1]) == "--help") {
        print_banner();
        print_readme();
        return 0;
    }

    if (argc != 3) {
        print_banner();
        std::cout << "[-] Error: Invalid arguments.\n";
        std::cout << "    Usage: scipio <binary_file> <output_file_name>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];

    std::cout << "[*] Target Binary : " << input_file << "\n";
    std::cout << "[*] Output Graph  : " << output_file << "\n";
    std::cout << "[*] Initializing Scipio Engine...\n\n";

    parse_elf_header(argv[1]);

    return 0;
}