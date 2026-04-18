#include "cfg_builder.hpp"

CFGBuilder::CFGBuilder(uint64_t entry) : entry_(entry) {
   
    leaders_.insert(entry);
}

void CFGBuilder::ingest(uint64_t address, const char* mnemonic, const char* operands) {
    instructions_.push_back(decode_instruction(address, mnemonic, operands));
}

CFG CFGBuilder::build() {
    collect_leaders();

    CFG cfg(entry_);
    assign_blocks(cfg);
    return cfg;  // NRVO
}