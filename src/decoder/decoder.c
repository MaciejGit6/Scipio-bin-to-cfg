#include "decoder.h"
#include <string.h>

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