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

#define REG_LRU_SIZE 18
int reg_lru[18];

enum CMM_MIPS_REG {
    REG_ERR = -1,
    REG_T0  = 0,
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

void gen_code_from_ir(LArrayOfIR_CODE* irs);


LString reg_to_string(enum CMM_MIPS_REG reg) {
    char buffer[10];
    if (reg < REG_S0) {
        sprintf(buffer, "$t%d", reg - REG_T0);
    } else {
        sprintf(buffer, "$s%d", reg - REG_S0);
    }
    return MakeLStringWith(buffer);
}


void init_available_registers() {
    memset(reg_lru, 0, sizeof(reg_lru));
    memset(used_regs.s, 0, sizeof(used_regs.s));
    memset(used_regs.t, 0, sizeof(used_regs.t));
}

void push_asm_line(LString str) { PushLArrayOfLString(GENERATED_ASM, str); }

/// @returns last recently used register id if poped, or itself if not poped
int mark_var_visited(int var_id) {
    int in_lru_index = REG_LRU_SIZE - 1;
    for (int i = 0; i < REG_LRU_SIZE; i++) {
        if (reg_lru[i] == var_id) {
            in_lru_index = i;
            break;
        }
    }

    int ret = reg_lru[in_lru_index];

    for (int i = in_lru_index; i > 0; i--) { reg_lru[i] = reg_lru[i - 1]; }
    reg_lru[0] = var_id;

    return ret;
}

int still_in_lru(int var_id) {
    for (int i = 0; i < REG_LRU_SIZE; i++) {
        if (reg_lru[i] == var_id) { return 1; }
    }
    return 0;
}

enum CMM_MIPS_REG find_first_available_register(int id) {
    for (int i = 0; i < 10; i++) {
        if (used_regs.t[i] == 0) {
            used_regs.t[i] = id;   // 标记为已使用
            return i + REG_T0;     // t0~t9 对应
        }
    }
    for (int i = 0; i < 8; i++) {
        if (used_regs.s[i] == 0) {
            used_regs.s[i] = id;   // 标记为已使用
            return i + REG_S0;     // s0~s7 对应 $s0~$s7
        }
    }
    return REG_ERR;
}

enum CMM_MIPS_REG find_current_register_for(int id) {
    for (int i = 0; i < 10; i++) {
        if (used_regs.t[i] == id) { return i + REG_T0; }   // t0~t9 对应
    }
    for (int i = 0; i < 8; i++) {
        if (used_regs.s[i] == id) { return i + REG_S0; }   // s0~s7 对应 $s0~$s7
    }
    return REG_ERR;
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
    push_asm_line(MakeLStringWith(CODEGEN_TEM));
    LArrayOfIR_CODE* irs = get_ir_output();

    gen_code_from_ir(irs);
    return 0;
}

enum CMM_MIPS_REG allocate_reg_for(CMM_IR_VAR var) {
    int id    = var.data.id;
    int poped = mark_var_visited(id);
    if (poped == id) {
        // still in lru
        return find_current_register_for(id);
    }
    enum CMM_MIPS_REG ret = find_current_register_for(poped);
    // todo
    return ret;
}

void gen_code_from_ir(LArrayOfIR_CODE* irs) {
    size_t irs_len = irs->size;
    char   buffer[50];

    for (size_t idx = 0; idx < irs_len; idx++) {
        IR_CODE ir = irs->data[idx];

        // 处理每条 IR 指令
        switch (ir.kind) {
            case IR_LABEL:
                sprintf(buffer, "label_%d:", ir.data.label.id);
                push_asm_line(MakeLStringWith(buffer));
                break;
            case IR_FUNCTION:
                sprintf(buffer, "%s:", ir.data.func_name);
                push_asm_line(MakeLStringWith(buffer));
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