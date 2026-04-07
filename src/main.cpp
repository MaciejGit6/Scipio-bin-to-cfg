#include <iostream>
#include <fstream>  
#include <string>
#include "elf_parser.h"

void print_readme() {
    std::ifstream file("README.md"); 
    if (file.is_open()) {
        std::string line;
        std::cout << "\n--- PROJECT DOCUMENTATION (README.md) ---\n" << std::endl;
        while (getline(file, line)) {
            std::cout << line << std::endl;
        }
        file.close();
        std::cout << "\n------------------------------------------\n" << std::endl;
    } else {
        std::cout << "[-] Error: README.md not found in the current directory." << std::endl;
        std::cout << "    Please ensure the file exists or check the project repository." << std::endl;
    }
}
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

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    

    if ( argv[1] && std::string(argv[1]) == "--help") {
        print_readme();
        return 0;
    }

    print_banner();

    
    return 0;
}