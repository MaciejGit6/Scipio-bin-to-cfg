#include "cfg.hpp"
#include <fstream>
#include <iostream>

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
    
    // --- LAYOUT UPGRADES ---
    // splines=ortho forces arrows to draw with sharp 90-degree angles (like a circuit board)
    // nodesep & ranksep add padding between the blocks so they don't overlap
    
    out << "    graph [splines=polyline, nodesep=0.5, ranksep=0.5];\n";
    
    // Changing fontname to "monospace" allows the web browser to dynamically calculate the box size!
    // Adding 'margin' gives the text breathing room so it doesn't touch the borders.
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