#ifndef CFG_HPP
#define CFG_HPP

#include "basic_block.hpp"
#include <unordered_map>
#include <memory>
#include <string>

class CFG {
private:
    // The core matrix: Maps a starting memory address to its BasicBlock
    std::unordered_map<uint64_t, std::shared_ptr<BasicBlock>> blocks;
    
    // The entry point of the function being analyzed
    uint64_t entry_address;

public:
    // Constructor requires the starting address of the function/binary
    CFG(uint64_t entry) : entry_address(entry) {}

    // Core Graph Operations
    std::shared_ptr<BasicBlock> get_or_create_block(uint64_t address);
    void add_edge(uint64_t source_addr, uint64_t target_addr);

    // Export the graph to the industry-standard Graphviz DOT format
    void export_to_dot(const std::string& filename);
};



#endif // CFG_HPP