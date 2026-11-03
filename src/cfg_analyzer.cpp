#include "cfg_analyzer.hpp"
#include <algorithm>
#include <queue>
#include <stack>
#include <unordered_map>

CFGAnalyzer::CFGAnalyzer(const CFG& cfg, uint64_t entry) 
    : cfg_(cfg), entry_(entry) {
}

// Blocks reachable from the entry point and the extra roots, following both
// control-flow edges and direct calls
std::unordered_set<uint64_t> CFGAnalyzer::bfs_reachable() const{
    std::unordered_set<uint64_t> visited;
    std::queue<uint64_t> worklist;

    auto visit = [&](uint64_t addr) {
        if (cfg_.get_block(addr) && visited.insert(addr).second)
            worklist.push(addr);
    };

    visit(entry_);
    for (uint64_t root : cfg_.get_roots())
        visit(root);

    while(!worklist.empty()){
        uint64_t addr = worklist.front();
        worklist.pop();

        auto block = cfg_.get_block(addr);

        for(const auto& succ : block->successors)
            visit(succ->start_address);
        for(uint64_t callee : block->callees)
            visit(callee);
    }
    return visited;
}

// Iterative DFS over every block (entry first, then by address) so loops are
// found in all functions and deep graphs can't overflow the stack
std::vector<std::pair<uint64_t, uint64_t>> CFGAnalyzer::dfs_back_edges() const{
    enum Color { WHITE, GREY, BLACK };
    std::unordered_map<uint64_t, Color> color;
    std::vector<std::pair<uint64_t, uint64_t>> back_edges;

    auto starts = cfg_.all_block_addresses();
    std::sort(starts.begin(), starts.end());
    starts.insert(starts.begin(), entry_);

    // Each frame: block address + index of the next successor to visit
    std::stack<std::pair<uint64_t, size_t>> frames;

    for (uint64_t start : starts) {
        if (!cfg_.get_block(start) || color[start] != WHITE) continue;

        color[start] = GREY;
        frames.push({start, 0});

        while (!frames.empty()) {
            auto& [addr, next] = frames.top();
            const BasicBlock* block = cfg_.get_block(addr);

            if (next == block->successors.size()) {
                color[addr] = BLACK;
                frames.pop();
                continue;
            }

            uint64_t s = block->successors[next++]->start_address;
            Color c = color[s];
            if (c == GREY) {
                back_edges.push_back({addr, s});
            } else if (c == WHITE) {
                color[s] = GREY;
                frames.push({s, 0});
            }
        }
    }
    return back_edges;
}

// Weakly connected components of the control-flow graph (≈ one per function)
int CFGAnalyzer::count_components() const {
    std::unordered_map<uint64_t, uint64_t> parent;
    auto find_root = [&](uint64_t x) {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    };

    auto addrs = cfg_.all_block_addresses();
    for (uint64_t addr : addrs) parent[addr] = addr;

    int components = static_cast<int>(addrs.size());
    for (uint64_t addr : addrs) {
        for (const auto& succ : cfg_.get_block(addr)->successors) {
            uint64_t a = find_root(addr), b = find_root(succ->start_address);
            if (a != b) {
                parent[a] = b;
                components--;
            }
        }
    }
    return components;
}

AnalysisReport CFGAnalyzer::analyze() const{
    AnalysisReport report;

    auto reachable = bfs_reachable();
    report.back_edges = dfs_back_edges();

    auto all_blocks = cfg_.all_block_addresses();
    std::sort(all_blocks.begin(), all_blocks.end());

    report.block_count = static_cast<int>(all_blocks.size());

    for(uint64_t addr : all_blocks){
        auto block = cfg_.get_block(addr);
        if(block)
            report.edge_count += static_cast<int>(block->successors.size());
    }

    // M = E - N + 2P; the binary holds many disjoint function graphs
    report.component_count = count_components();
    report.cyclomatic_complexity =
        report.edge_count - report.block_count + 2 * report.component_count;

    for (uint64_t addr : all_blocks) {
        if (reachable.count(addr))
            report.reachable_blocks.push_back(addr);
        else
            report.unreachable_blocks.push_back(addr);
    }

    std::unordered_set<uint64_t> headers;
    for (const auto& [from, to] : report.back_edges)
        if (headers.insert(to).second)
            report.loop_headers.push_back(to);
    std::sort(report.loop_headers.begin(), report.loop_headers.end());

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
