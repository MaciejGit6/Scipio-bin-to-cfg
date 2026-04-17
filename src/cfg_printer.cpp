#include "cfg_printer.hpp"
#include <algorithm>

std::string CFGPrinter::node_style(uint64_t addr) const {
    if (addr == entry_)
        return "style=filled fillcolor=\"#a8d8a8\" color=\"#2d6a2d\""; // green

    bool is_loop = std::find(report_.loop_headers.begin(),
                             report_.loop_headers.end(), addr)
                   != report_.loop_headers.end();
    if (is_loop)
        return "style=filled fillcolor=\"#f4a261\" color=\"#8b3a00\""; // orange

    bool is_dead = std::find(report_.unreachable_blocks.begin(),
                             report_.unreachable_blocks.end(), addr)
                   != report_.unreachable_blocks.end();
    if (is_dead)
        return "style=filled fillcolor=\"#cccccc\" color=\"#888888\" fontcolor=\"#888888\"";

    return "style=filled fillcolor=\"#dde8f5\" color=\"#2c5282\""; // default blue
}

std::string CFGPrinter::block_label(const BasicBlock* block) const {
    std::ostringstream ss;
    ss << std::hex << "0x" << block->start_address;
    ss << "\\n[" << std::dec << block->instructions.size() << " insns]";

    
    uint64_t addr = block->start_address;
    if (addr == entry_)   ss << "\\n<ENTRY>";
    
    bool is_loop = std::find(report_.loop_headers.begin(),
                             report_.loop_headers.end(), addr)
                   != report_.loop_headers.end();
    if (is_loop) ss << "\\n<LOOP HEADER>";

    return ss.str();
}