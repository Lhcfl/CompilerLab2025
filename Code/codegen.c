#include "codegen.h"
#include "ir.h"
#include "llib.h"
#include <stdio.h>

LArrayOfLString* GENERATED_ASM = NULL;
char             codegen_buffer_ptr[512];

/// 包含 read 和 write 系统调用的模板
const char* CODEGEN_TEM = ".data \n"
                          "_prompt: .asciiz \"Enter an integer:\" \n"
                          "_ret: .asciiz \"\\n\" \n"
                          ".globl main \n"
                          ".text \n"
                          "read: \n"
                          "  li $v0, 4 \n"
                          "  la $a0, _prompt \n"
                          "  syscall \n"
                          "  li $v0, 5 \n"
                          "  syscall \n"
                          "  jr $ra \n"
                          "write: \n"
                          "  li $v0, 1 \n"
                          "  syscall \n"
                          "  li $v0, 4 \n"
                          "  la $a0, _ret \n"
                          "  syscall \n"
                          "  move $v0, $0 \n"
                          "  jr $ra \n";

// 主翻译过程
int cmm_gen_code() {
    GENERATED_ASM = NewLArrayOfLString();
    PushLArrayOfLString(GENERATED_ASM, MakeLStringWith(CODEGEN_TEM));
    // LArrayOfLString* irs    = get_ir_output();
    // size_t           ir_len = get_ir_output()->size;

    // for (size_t idx = 0; idx < ir_len; idx++) { LString ir = irs->data[idx];
    // }

    return 0;
}

inline LArrayOfLString* get_asm_output() { return GENERATED_ASM; }
void                    free_asm_output() {
    FreeLArrayOfLString(GENERATED_ASM);
    GENERATED_ASM = NULL;
}