#include "decoder.h"
#include <string.h>
#include <stdlib.h>

// Capstone reports prefixes as part of the mnemonic ("bnd jmp", "notrack jmp",
// "repz ret"), so classify on the last word only.
static const char* base_mnemonic(const char* mnemonic) {
    const char* space = strrchr(mnemonic, ' ');
    return space ? space + 1 : mnemonic;
}

InsnType classify_instruction(const char* mnemonic){
    mnemonic = base_mnemonic(mnemonic);

    if(strcmp(mnemonic, "ret") == 0 || strcmp(mnemonic, "retq") == 0 ||
       strcmp(mnemonic, "retf") == 0)
        return INSN_RET;

    // No fall-through: execution never continues to the next instruction
    if(strcmp(mnemonic, "hlt") == 0 || strcmp(mnemonic, "ud2") == 0)
        return INSN_RET;

    if(strcmp(mnemonic, "call") == 0)
        return INSN_CALL;

    if(strcmp(mnemonic, "jmp") == 0)
        return INSN_BRANCH_UNCONDITIONAL;

    if (mnemonic[0] == 'j' || strncmp(mnemonic, "loop", 4) == 0)
        return INSN_BRANCH_CONDITIONAL;

    return INSN_NORMAL;
}

// Only direct branches have an immediate operand ("0x401020"). Indirect forms
// like "qword ptr [rip + 0x2fe2]" or "rax" have no static target.
static uint64_t extract_branch_target(const char* operands) {
    if (strncmp(operands, "0x", 2) != 0)
        return 0;

    char* end;
    uint64_t target = strtoull(operands, &end, 16);

    if (end == operands || *end != '\0')
        return 0;

    return target;
}


DecodedInsn decode_instruction(uint64_t address,
                               const char* mnemonic,
                               const char* operands)
{
    DecodedInsn out;
    memset(&out, 0, sizeof(out));
    out.address = address;
    out.type    = classify_instruction(mnemonic);
    out.target  = 0;

    strncpy(out.mnemonic, mnemonic, sizeof(out.mnemonic) - 1);
    out.mnemonic[sizeof(out.mnemonic) - 1] = '\0';

    strncpy(out.operands, operands, sizeof(out.operands) - 1);
    out.operands[sizeof(out.operands) - 1] = '\0';

    if (out.type != INSN_NORMAL && out.type != INSN_RET)
        out.target = extract_branch_target(operands);

    return out;
}
