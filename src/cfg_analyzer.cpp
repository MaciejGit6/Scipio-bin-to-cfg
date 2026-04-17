#include "cfg_analyzer.hpp"
#include <queue>
#include <functional>

CFGAnalyzer::CFGAnalyzer(const CFG& cfg, uint64_t entry) 
    : cfg_(cfg), entry_(entry) {
}

std::unordered_set<uint64_t> CFGAnalyzer::bfs_reachable() const{
    std::unordered_set<uint64_t> visited;
    std::queue<uint64_t> worklist;

    worklist.push(entry_);
    visited.insert(entry_);

    while(!worklist.empty()){
        uint64_t addr = worklist.front();
        worklist.pop();

        auto block = cfg_.get_block(addr);
        if(!block) continue;

        for(const auto& succ : block->successors){
            if(visited.insert(succ->start_address).second)
                worklist.push(succ->start_address);
        }
    }
    return visited;
}

std::unordered_set<uint64_t> CFGAnalyzer::dfs_back_edges() const{
    std::unordered_set<uint64_t> grey, black, loop_headers;

    std::function<void(uint64_t)> dfs = [&](uint64_t addr){
        grey.insert(addr);

        auto block = cfg_.get_block(addr);
        if(!block){grey.erase(addr); black.insert(addr); return;}

        for(const auto& succ : block->successors){
            uint64_t s = succ->start_address;
            if(grey.count(s)){
                loop_headers.insert(s);

            }else if (!black.count(s)){
                dfs(s);
            }
        }
        grey.erase(addr);
        black.insert(addr);
    };

    dfs(entry_);
    return loop_headers;
}

AnalysisReport CFGAnalyzer::analyze() const{
    AnalysisReport report;

    auto reachable = bfs_reachable();
    auto back_targets = dfs_back_edges();

    auto all_blocks = cfg_.all_block_addresses();

    report.block_count = static_cast<int>(all_blocks.size());

    for(uint64_t addr : all_blocks){
        auto block = cfg_.get_block(addr);
        if(block)
            report.edge_count += static_cast<int>(block->successors.size());


    }
    report.cyclomatic_complexity = report.edge_count - report.block_count + 2;

    for (uint64_t addr : all_blocks) {
        if (reachable.count(addr))
            report.reachable_blocks.push_back(addr);
        else
            report.unreachable_blocks.push_back(addr);
    }

    report.loop_headers.assign(back_targets.begin(), back_targets.end());

    return report;
}


const BasicBlock* CFG::get_block(uint64_t address) const {
    auto it = blocks.find(address);
    return (it != blocks.end()) ? it->second.get() : nullptr;
}

std::vector<uint64_t> CFG::all_block_addresses() const {
    std::vector<uint64_t> addrs;
    addrs.reserve(blocks.size());
    for (const auto& [addr, _] : blocks) 
        addrs.push_back(addr);
    return addrs;
}