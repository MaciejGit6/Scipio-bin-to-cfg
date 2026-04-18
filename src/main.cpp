
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>  // Required for readlink()
#include <limits.h>  // Required for PATH_MAX
#include <cstdlib>
#include "elf_parser.h"
#include "cfg_builder.hpp"

#include <memory>
#include "cfg.hpp"
#include "decoder.h"
#include "cfg_analyzer.hpp"
#include "cfg_printer.hpp"



std::unique_ptr<CFGBuilder> builder = nullptr;

extern "C" void instruction_receiver(uint64_t address,
                                     const char* mnemonic,
                                     const char* operands) {
    builder->ingest(address, mnemonic, operands);
}







void print_banner() {
    /*std::cout << R"(
   _____  _____ _____ _____ _____  ____  
  / ____|/ ____|_   _|  __ \_   _|/ __ \ 
 | (___ | |      | | | |__) || | | |  | |
  \___ \| |      | | |  ___/ | | | |  | |
  ____) | |____ _| |_| |    _| |_| |__| |
 |_____/ \_____|_____|_|   |_____|\____/ 
                                         
    )" << '\n';*/
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

        uint64_t entry = get_elf_entry(input_file.c_str());
        builder = std::make_unique<CFGBuilder>(entry);

        disassemble_text_section(input_file.c_str(), instruction_receiver);

        auto graph = builder->build();

        CFGAnalyzer analyzer(graph, entry);
        auto report = analyzer.analyze();

        std::cout << "\n[*] CFG Analysis Report\n";
        std::cout << "    Blocks            : " << report.block_count << "\n";
        std::cout << "    Edges             : " << report.edge_count << "\n";
        std::cout << "    Cyclomatic Cmplx  : " << report.cyclomatic_complexity << "\n";
        std::cout << "    Dead blocks       : " << report.unreachable_blocks.size() << "\n";

        if (!report.loop_headers.empty()) {
            std::cout << "    Loop headers      : ";
            for (uint64_t addr : report.loop_headers)
                std::cout << std::hex << "0x" << addr << " ";
            std::cout << "\n";
        }

        CFGPrinter printer(graph, report, entry);
        printer.export_annotated_dot(output_file);
        printer.print_ascii_summary();

        std::cout << "[*] Running Graphviz to generate PDF...\n";

        std::string pdf_file = output_file;
        size_t dot_pos = pdf_file.find_last_of('.');
        if (dot_pos != std::string::npos)
            pdf_file = pdf_file.substr(0, dot_pos) + ".pdf";
        else
            pdf_file += ".pdf";

        std::string command = "dot -Tpdf " + output_file + " -o " + pdf_file;
        int result = std::system(command.c_str());

        if (result == 0) {
            std::cout << "[+] Visualization ready! You can view it by running:\n";
            std::cout << "    explorer.exe " << pdf_file << "\n";
        } else {
            std::cout << "[-] Error: Graphviz rendering failed.\n";
            std::cout << "    Make sure it is installed: sudo apt install graphviz\n";
        }
    }

    return 0;
}