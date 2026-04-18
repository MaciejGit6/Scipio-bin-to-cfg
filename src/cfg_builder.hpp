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

void CFGBuilder::assign_blocks(CFG& cfg) const {
    std::shared_ptr<BasicBlock> current = nullptr;

    for (size_t i = 0; i < instructions_.size(); i++) {
        const DecodedInsn& insn = instructions_[i];

        if (leaders_.count(insn.address)) {
            if (current != nullptr) {
  
                cfg.add_edge(current->start_address, insn.address);
            }
            current = cfg.get_or_create_block(insn.address);
        }

        if (current == nullptr)
            current = cfg.get_or_create_block(insn.address);

        current->add_instruction(insn.address, insn.mnemonic, insn.operands);

        if (insn.type != INSN_NORMAL) {
            if (insn.target != 0)
                cfg.add_edge(current->start_address, insn.target);
            current = nullptr;
        }
    }
}