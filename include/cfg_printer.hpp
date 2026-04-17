#pragma once
#include "cfg.hpp"
#include "cfg_analyzer.hpp"
#include <string>

class CFGPrinter {
public:
    CFGPrinter(const CFG& cfg, const AnalysisReport& report, uint64_t entry);

    void export_annotated_dot(const std::string& filename) const;

    
    void print_ascii_summary() const;

private:
    const CFG&            cfg_;
    const AnalysisReport& report_;
    uint64_t              entry_;

    std::string node_style(uint64_t addr) const;
    std::string block_label(const BasicBlock* block) const;
};