
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>  // Required for readlink()
#include <limits.h>  // Required for PATH_MAX
#include <cstdlib>
#include "elf_parser.h"

#include <memory>
#include "cfg.hpp"

std::unique_ptr<CFG> graph_engine = nullptr;
std::shared_ptr<BasicBlock> current_block = nullptr;


extern "C" void instruction_receiver(uint64_t address, const char* mnemonic, const char* operands) {
    // If we don't have an active block, create one at the current address
    if (current_block == nullptr) {
        current_block = graph_engine->get_or_create_block(address);
    }

    // Add the instruction to the block
    current_block->add_instruction(address, mnemonic, operands);

    // --- THE BLOCK SPLITTER LOGIC ---
    std::string mnem(mnemonic);
    std::string ops(operands);

    // Is this a branch/jump instruction? 
    // (Starts with 'j', or is 'call', or is 'ret')
    bool is_branch = (mnem[0] == 'j' || mnem == "call" || mnem == "ret");

    if (is_branch) {
        // If it jumps to a specific hex address, extract it and create an edge!
        size_t hex_pos = ops.find("0x");
        if (hex_pos != std::string::npos) {
            try {
                // Convert the hex string to an actual integer
                uint64_t target_addr = std::stoull(ops.substr(hex_pos), nullptr, 16);
                
                // Add the edge to the Adjacency Matrix
                graph_engine->add_edge(current_block->start_address, target_addr);
            } catch (...) {
                // Ignore parsing errors (e.g., if the jump is to a register like `call rax`)
            }
        }

        // Close the block! The NEXT instruction will trigger the creation of a new BasicBlock.
        current_block = nullptr; 
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

    if (parse_elf_header(input_file.c_str()) == 0) {
        
        // 3. Initialize the CFG (We will assume entry is 0 for now)
        graph_engine = std::make_unique<CFG>(0);

        // 4. Run the C decoder, passing our C++ receiver function!
        disassemble_text_section(input_file.c_str(), instruction_receiver);

        // 5. Export the populated graph to a .dot file
        graph_engine->export_to_dot(output_file);

        // 6. Automatically generate the SVG file
        std::cout << "[*] Running Graphviz to generate SVG...\n";
        
        // Figure out the SVG filename (replace .cfg with .svg)
        std::string svg_file = output_file;
        size_t dot_pos = svg_file.find_last_of('.');
        if (dot_pos != std::string::npos) {
            svg_file = svg_file.substr(0, dot_pos) + ".svg";
        } else {
            svg_file += ".svg";
        }

        // Construct the terminal command
        std::string command = "dot -Tsvg " + output_file + " -o " + svg_file;
        
        // Execute the command in the Linux shell
        int result = std::system(command.c_str());
        
        if (result == 0) {
            std::cout << "[+] Visualization ready! You can view it by running:\n";
            std::cout << "    explorer.exe " << svg_file << "\n";
        } else {
            std::cout << "[-] Error: Graphviz rendering failed.\n";
            std::cout << "    Make sure it is installed: sudo apt install graphviz\n";
        }
    }

    return 0;
}