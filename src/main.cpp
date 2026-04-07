#include <iostream>

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
    print_banner();
    
    return 0;
}