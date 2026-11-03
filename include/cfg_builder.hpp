#pragma once
#include "cfg.hpp"
#include "decoder.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class CFGBuilder {
public:
    explicit CFGBuilder(uint64_t entry);

    void ingest(uint64_t address, const char* mnemonic, const char* operands);
    void add_function(uint64_t address, const char* name);

    [[nodiscard]] CFG build();

private:
    uint64_t                     entry_;
    std::vector<DecodedInsn>     instructions_;
    std::unordered_set<uint64_t> insn_addresses_;
    std::unordered_set<uint64_t> leaders_;
    std::unordered_map<uint64_t, std::string> functions_;   // start address -> name

    void collect_leaders();
    void assign_blocks(CFG& cfg) const;
};
