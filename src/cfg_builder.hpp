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

void CFGBuilder::collect_leaders() {
    for (size_t i = 0; i < instructions_.size(); i++) {
        const DecodedInsn& insn = instructions_[i];

        if (insn.type == INSN_NORMAL) continue;

    
        if (insn.target != 0)
            leaders_.insert(insn.target);

        bool is_conditional = (insn.type == INSN_BRANCH_CONDITIONAL);
        bool has_next       = (i + 1 < instructions_.size());

        if (is_conditional && has_next)
            leaders_.insert(instructions_[i + 1].address);
    }
}