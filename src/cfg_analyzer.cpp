#include "cfg_analyzer.hpp"
#include <queue>

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

std_unordered_set<uint64_t> CFGAnalyzer::dfs_back_edges() const{
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