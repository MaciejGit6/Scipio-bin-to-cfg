#include "decoder.h"
#include <string.h>
#include <stdlib.h>

InsnType classify_instruction(const char* mnemonic){
    if(strcmp(mnemonic, "ret") == 0 || strcmp(mnemonic, "retq") == 0)
        return INSN_RET;

    if(strcmp(mnemonic, "call") == 0)
    return INSN_CALL;

    if(strcmp(mnemonic, "jmp") == 0)
        return INSN_BRANCH_UNCONDITIONAL;

    if (mnemonic[0] == 'j')
        return INSN_BRANCH_CONDITIONAL;

    return INSN_NORMAL;

            
}

static uint64_t extract_branch_target(const char* operands) {
    const char* hex = strstr(operands, "0x");
    if (hex == NULL)
        return 0;

    char* end;
    uint64_t target = strtoull(hex, &end, 16);

  
    if (end == hex)
        return 0;

    return target;
}