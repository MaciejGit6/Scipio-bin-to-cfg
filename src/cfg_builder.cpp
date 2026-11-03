#include "cfg_builder.hpp"

CFGBuilder::CFGBuilder(uint64_t entry) : entry_(entry) {
    leaders_.insert(entry);
}

void CFGBuilder::ingest(uint64_t address, const char* mnemonic, const char* operands) {
    instructions_.push_back(decode_instruction(address, mnemonic, operands));
    insn_addresses_.insert(address);
}

void CFGBuilder::add_function(uint64_t address, const char* name) {
    functions_[address] = name;
    leaders_.insert(address);
}

CFG CFGBuilder::build() {
    collect_leaders();
    CFG cfg(entry_);

    // libc calls main through a function pointer, so it can't be found from _start
    for (const auto& [addr, name] : functions_)
        if (name == "main" && insn_addresses_.count(addr))
            cfg.add_root(addr);

    assign_blocks(cfg);
    return cfg;
}

void CFGBuilder::collect_leaders() {
    for (size_t i = 0; i < instructions_.size(); i++) {
        const DecodedInsn& insn = instructions_[i];

        if (insn.type == INSN_NORMAL || insn.type == INSN_RET) continue;

        // Call targets are function starts, so they begin a block too
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
    std::string current_func;

    for (size_t i = 0; i < instructions_.size(); i++) {
        const DecodedInsn& insn = instructions_[i];

        auto func_it = functions_.find(insn.address);
        if (func_it != functions_.end())
            current_func = func_it->second;

        if (leaders_.count(insn.address)) {
            // Code doesn't fall through into another function; the previous
            // block most likely ended in a call that never returns
            bool is_func_start = func_it != functions_.end();
            if (current != nullptr && !is_func_start)
                cfg.add_edge(current->start_address, insn.address);
            current = cfg.get_or_create_block(insn.address);
        }

        if (current == nullptr)
            current = cfg.get_or_create_block(insn.address);

        if (current->instructions.empty())
            current->func = current_func;

        current->add_instruction(insn.address, insn.mnemonic, insn.operands);

        // Targets outside the disassembled code (e.g. PLT stubs) would only
        // create empty phantom blocks
        bool target_known = insn.target != 0 && insn_addresses_.count(insn.target);

        switch (insn.type) {
            case INSN_BRANCH_UNCONDITIONAL:
                if (target_known)
                    cfg.add_edge(current->start_address, insn.target);
                current = nullptr;            
                break;

            case INSN_BRANCH_CONDITIONAL:
                if (target_known)
                    cfg.add_edge(current->start_address, insn.target);
               
                break;

            case INSN_CALL:
                if (target_known)
                    current->callees.push_back(insn.target);
                break;

            case INSN_RET:
                current = nullptr;            
                break;

            default:                          
                break;
        }
    }
}
