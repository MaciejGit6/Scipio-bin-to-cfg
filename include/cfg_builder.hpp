#pragma once
#include "cfg.hpp"
#include "decoder.h"
#include <vector>
#include <unordered_set>

class CFGBuilder {
public:
    explicit CFGBuilder(uint64_t entry);

    void ingest(uint64_t address, const char* mnemonic, const char* operands);

    [[nodiscard]] CFG build();

private:
    uint64_t entry_t;
    std::vector<DecodedInsn> instructions_;
    std::unordered_set<uint64_t> leaders_;

    void collect_leaders();
    void assign_blocks(CFG& cfg) const;
};




