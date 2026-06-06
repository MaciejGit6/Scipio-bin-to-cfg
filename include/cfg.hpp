#ifndef CFG_HPP
#define CFG_HPP

#include "basic_block.hpp"
#include <unordered_map>
#include <memory>
#include <string>

class CFG {
private:
    
    std::unordered_map<uint64_t, std::shared_ptr<BasicBlock>> blocks;
    
    
    uint64_t entry_address;

public:

    const BasicBlock* get_block(uint64_t address) const;
    std::vector<uint64_t> all_block_addresses() const;
    uint64_t get_entry() const { return entry_address; }
        
    CFG(uint64_t entry) : entry_address(entry) {}

    // Core Graph Operations
    std::shared_ptr<BasicBlock> get_or_create_block(uint64_t address);
    void add_edge(uint64_t source_addr, uint64_t target_addr);

    // Export the graph to the industry-standard Graphviz DOT format
    void export_to_dot(const std::string& filename);
    void export_to_json(const std::string& filename) const;
};



#endif // CFG_HPP