#ifndef DECODER_H
#define DECODER_H
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef enum{
    INSN_NORMAL, //mov, add, push
    INSN_CALL, //call <target>
    INSN_BRANCH_UNCONDITIONAL, //jmp
    INSN_BRANCH_CONDITIONAL, //jne, jz, jg
    INSN_RET // ret, retq
}InsnType;

typedef struct{
    uint64_t address;
    char mnemonic[32];
    char operands[128];
    InsnType type;
    uint64_t target;
} DecodedInsn;

#ifdef __cplusplus
}
#endif

#endif