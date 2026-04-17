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

void CFGPrinter::export_annotated_dot(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "[-] Could not open: " << filename << "\n";
        return;
    }

    out << "digraph CFG {\n";
    out << "    graph [fontname=\"Courier\" bgcolor=\"#1a1a2e\"];\n";
    out << "    node  [fontname=\"Courier\" shape=box];\n";
    out << "    edge  [fontname=\"Courier\" color=\"#aaaaaa\"];\n\n";

    for (uint64_t addr : cfg_.all_block_addresses()) {
        const BasicBlock* block = cfg_.get_block(addr);
        if (!block) continue;

        out << "    \"0x" << std::hex << addr << "\" ["
            << "label=\"" << block_label(block) << "\" "
            << node_style(addr) << "];\n";

        for (const auto& succ : block->successors) {
            uint64_t s = succ->start_address;

            
            bool is_back_edge = s <= addr &&
                std::find(report_.loop_headers.begin(),
                          report_.loop_headers.end(), s)
                != report_.loop_headers.end();

            out << "    \"0x" << std::hex << addr
                << "\" -> \"0x" << s << "\"";
            if (is_back_edge)
                out << " [color=\"#e63946\" style=dashed label=\"back\"]";
            out << ";\n";
        }
    }

    out << "}\n";
    std::cout << "[+] Annotated graph written to: " << filename << "\n";
}

void CFGPrinter::print_ascii_summary() const {
    constexpr int BAR_MAX = 40;

    // find the largest block so we can scale bars
    int max_insns = 1;
    for (uint64_t addr : cfg_.all_block_addresses()) {
        const BasicBlock* b = cfg_.get_block(addr);
        if (b) max_insns = std::max(max_insns, (int)b->instructions.size());
    }

    std::cout << "\n[*] Block size distribution\n";
    std::cout << "    " << std::string(BAR_MAX + 20, '-') << "\n";

    for (uint64_t addr : report_.reachable_blocks) {
        const BasicBlock* b = cfg_.get_block(addr);
        if (!b) continue;

        int bar_len = (b->instructions.size() * BAR_MAX) / max_insns;
        bar_len = std::max(bar_len, 1);

        std::cout << "    0x" << std::hex << std::setw(8) << addr << "  |"
                  << std::string(bar_len, '#')
                  << std::string(BAR_MAX - bar_len, ' ')
                  << "| " << std::dec << b->instructions.size() << "\n";
    }

    if (!report_.unreachable_blocks.empty()) {
        std::cout << "\n    [dead code]\n";
        for (uint64_t addr : report_.unreachable_blocks)
            std::cout << "    0x" << std::hex << addr << "  |"
                      << std::string(BAR_MAX, 'x') << "| unreachable\n";
    }

    std::cout << "    " << std::string(BAR_MAX + 20, '-') << "\n";
}