#include "cfg.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

// Retrieves an existing block, or creates a new one if it hasn't been discovered yet
std::shared_ptr<BasicBlock> CFG::get_or_create_block(uint64_t address) {
    if (blocks.find(address) == blocks.end()) {
        blocks[address] = std::make_shared<BasicBlock>(address);
    }
    return blocks[address];
}

// Creates a directional edge between two blocks
void CFG::add_edge(uint64_t source_addr, uint64_t target_addr) {
    auto source_block = get_or_create_block(source_addr);
    auto target_block = get_or_create_block(target_addr);
    
    // Add the target to the source's successors (The Adjacency List)
    source_block->successors.push_back(target_block);
}

// Exports the graph matrix to a visualizable DOT file
void CFG::export_to_dot(const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "[-] Error: Could not open output file " << filename << "\n";
        return;
    }

    out << "digraph CFG {\n";
    
    
    
    out << "    graph [splines=polyline, nodesep=0.5, ranksep=0.5];\n";
    
   
    out << "    node [shape=box, fontname=\"monospace\", fontsize=12, style=\"filled\", fillcolor=\"#fdfdfd\", margin=0.15, penwidth=1.2, color=\"#555555\"];\n";
    out << "    edge [color=\"#333333\", penwidth=1.5, arrowsize=0.8];\n\n";

    // 1. First Pass: Draw all the blocks
    for (const auto& pair : blocks) {
        auto block = pair.second;
        
        out << "    block_" << std::hex << block->start_address << " [label=\"";
        out << "loc_" << std::hex << block->start_address << ":\\l";
        
        for (const auto& insn : block->instructions) {
            out << "  " << insn.mnemonic << " " << insn.operands << "\\l";
        }
        out << "\"];\n";
    }

    out << "\n    // Edges\n";

    // 2. Second Pass: Draw the arrows connecting the blocks
    for (const auto& pair : blocks) {
        auto block = pair.second;
        for (const auto& succ : block->successors) {
            out << "    block_" << std::hex << block->start_address 
                << " -> block_" << std::hex << succ->start_address << ";\n";
        }
    }

    out << "}\n";
    std::cout << "[+] CFG successfully exported to " << filename << "\n";
}

void CFG::export_to_json(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "[-] Error: Could not open output file " << filename << "\n";
        return;
    }

  
    std::vector<uint64_t> addrs;
    addrs.reserve(blocks.size());
    for (const auto& [addr, _] : blocks)
        addrs.push_back(addr);
    std::sort(addrs.begin(), addrs.end());

    out << "{\n";
    out << "  \"entry\": \"0x" << std::hex << entry_address << "\",\n";

    out << "  \"blocks\": [\n";
    for (size_t i = 0; i < addrs.size(); ++i) {
        const BasicBlock* b = blocks.at(addrs[i]).get();
        out << "    {\"addr\": \"0x" << std::hex << addrs[i]
            << "\", \"insns\": " << std::dec << b->instructions.size() << "}"
            << (i + 1 < addrs.size() ? ",\n" : "\n");
    }
    out << "  ],\n";

    out << "  \"edges\": [\n";
    bool first = true;
    for (uint64_t addr : addrs) {
        const BasicBlock* b = blocks.at(addr).get();
        for (const auto& succ : b->successors) {
            if (!first) out << ",\n";
            out << "    {\"from\": \"0x" << std::hex << addr
                << "\", \"to\": \"0x" << succ->start_address << "\"}";
            first = false;
        }
    }
    out << "\n  ]\n}\n";

    std::cout << "[+] CFG exported to JSON: " << filename << "\n";
}