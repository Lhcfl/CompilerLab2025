#include "codegen.h"
#include "ir.h"
#include "llib.h"
#include "predefines.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

LArrayOfLString* GENERATED_ASM = NULL;
char             codegen_buffer_ptr[512];

struct {
    int t[10];   // t0~t9 the caller save regs
    int s[8];    // s0~s7 the callee save regs
} used_regs;

enum CMM_MIPS_REG {
    REG_T0 = 0,
    REG_T1,
    REG_T2,
    REG_T3,
    REG_T4,
    REG_T5,
    REG_T6,
    REG_T7,
    REG_T8,
    REG_T9,
    REG_S0 = 10,
    REG_S1,
    REG_S2,
    REG_S3,
    REG_S4,
    REG_S5,
    REG_S6,
    REG_S7,
};

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

void init_available_registers() {
    memset(used_regs.s, 0, sizeof(used_regs.s));
    memset(used_regs.t, 0, sizeof(used_regs.t));
}

enum CMM_MIPS_REG find_first_available_register() {
    for (int i = 0; i < 10; i++) {
        if (used_regs.t[i] == 0) {
            used_regs.t[i] = 1;   // 标记为已使用
            return i + REG_T0;    // t0~t9 对应
        }
    }
    for (int i = 0; i < 8; i++) {
        if (used_regs.s[i] == 0) {
            used_regs.s[i] = 1;   // 标记为已使用
            return i + REG_S0;    // s0~s7 对应 $s0~$s7
        }
    }
    cmm_panic("Error: No available registers found.\n");
}

void free_register(enum CMM_MIPS_REG reg) {
    if (reg < REG_T0 || reg > REG_S7) {
        cmm_panic("Error: Invalid register number %d.\n", reg);
    }
    if (reg < REG_S0) {
        used_regs.t[reg - REG_T0] = 0;   // 标记为可用
    } else {
        used_regs.s[reg - REG_S0] = 0;   // 标记为可用
    }
}

// 主翻译过程
int cmm_gen_code() {
    init_available_registers();
    GENERATED_ASM = NewLArrayOfLString();
    PushLArrayOfLString(GENERATED_ASM, MakeLStringWith(CODEGEN_TEM));
    LArrayOfIR_CODE* irs = get_ir_output();

    return 0;
}

enum CMM_MIPS_REG gen_reg_for(CMM_IR_VAR var) { return var.data.id; }

void gen_code_from_ir(LArrayOfIR_CODE* irs) {
    size_t irs_len = irs->size;
    char   buffer[50];

    for (size_t idx = 0; idx < irs_len; idx++) {
        IR_CODE ir = irs->data[idx];

        // 处理每条 IR 指令
        switch (ir.kind) {
            case IR_LABEL:
                sprintf(buffer, "label_%d:", ir.data.label.id);
                PushLArrayOfLString(GENERATED_ASM, MakeLStringWith(buffer));
                break;
            case IR_FUNCTION:
                sprintf(buffer, "%s:", ir.data.func_name);
                PushLArrayOfLString(GENERATED_ASM, MakeLStringWith(buffer));
                break;
            case IR_WRITE: cmm_panic("not impl"); break;
            case IR_READ: cmm_panic("not impl"); break;
            case IR_CALL: cmm_panic("not impl"); break;
            case IR_GOTO: cmm_panic("not impl"); break;
            case IR_RETURN: cmm_panic("not impl"); break;
            case IR_IF: cmm_panic("not impl"); break;
            case IR_2CODE: cmm_panic("not impl"); break;
            case IR_3CODE: cmm_panic("not impl"); break;
            case IR_ARG: cmm_panic("not impl"); break;
            case IR_PARAM: cmm_panic("not impl"); break;
            case IR_ARG_ADDR: cmm_panic("not impl"); break;
            case IR_ALLOC: cmm_panic("not impl"); break;
        }
    }
}

inline LArrayOfLString* get_asm_output() { return GENERATED_ASM; }
void                    free_asm_output() {
    FreeLArrayOfLString(GENERATED_ASM);
    GENERATED_ASM = NULL;
}