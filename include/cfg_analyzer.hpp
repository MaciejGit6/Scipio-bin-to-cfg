#pragma once 
#include "cfg.hpp"
#include <unordered_set>
#include <vector>
#include <optional>

struct AnalysisReport{
    int block_count = 0;
    int edge_count = 0;
    int cyclomatic_complexity = 0;

    std::vector<uint64_t> reachable_blocks;
    std::vector<uint64_t> unreachable_blocks;
    std::vector<uint64_t> loop_headers;


};

class CFGAnalyzer {
public:
    explicit CFGAnalyzer(const CFG& cfg, uint64_t entry);

    [[nodiscard]] AnalysisReport analyze() const;
private: 
    const CFG& cfg_;
    uint64_t entry_;

    std::unordered_set<uint64_t> bfd_reachable() const;
    std::unordered_set<uint64_t> dfs_back_edges() const;

}