#include "codegen.h"
#include "ir.h"
#include "llib.h"
#include "predefines.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#define PUSH_LSTR_ASM(...)                      \
    {                                           \
        sprintf(buffer, __VA_ARGS__);           \
        push_asm_line(MakeLStringWith(buffer)); \
    }

LArrayOfLString* GENERATED_ASM = NULL;
char             codegen_buffer_ptr[512];
char             buffer[500];

struct {
    int t[10];   // t0~t9 the caller save regs
    int s[8];    // s0~s7 the callee save regs
} used_regs;

#define REG_LRU_SIZE 18
int reg_lru[18];

typedef enum CMM_MIPS_REG {
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
} CMM_MIPS_REG;

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


LString reg_to_string(CMM_MIPS_REG reg) {
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

void push_stack_frame() { PUSH_LSTR_ASM("  addi $sp, $sp, -4"); }
void pop_stack_frame() { PUSH_LSTR_ASM("  addi $sp, $sp, 4"); }

void save_function_call_register() {
    push_stack_frame();
    PUSH_LSTR_ASM("  sw $ra, 0($sp)");
}

void recover_function_call_register() {
    PUSH_LSTR_ASM("  lw $ra, 0($sp)");
    pop_stack_frame();
}


void save_callee_registers() {
    PUSH_LSTR_ASM("  addi $sp, $sp, -32");
    for (int i = 0; i < 8; i++) {
        if (used_regs.s[i] != 0) {
            PUSH_LSTR_ASM("  sw $s%d, %d($sp)", i, i * 4);
        }
    }
}

void recover_callee_registers() {
    for (int i = 0; i < 8; i++) {
        if (used_regs.s[i] != 0) {
            PUSH_LSTR_ASM("  lw $s%d, %d($sp)", i, i * 4);
        }
    }
    PUSH_LSTR_ASM("  addi $sp, $sp, 32");
}

void save_caller_registers() {
    save_callee_registers();
    PUSH_LSTR_ASM("  addi $sp, $sp, -40");
    for (int i = 0; i < 10; i++) {
        if (used_regs.t[i] != 0) {
            PUSH_LSTR_ASM("  sw $t%d, %d($sp)", i, i * 4);
        }
    }
    save_function_call_register();
}

void recover_caller_registers() {
    recover_function_call_register();
    for (int i = 0; i < 10; i++) {
        if (used_regs.t[i] != 0) {
            PUSH_LSTR_ASM("  lw $t%d, %d($sp)", i, i * 4);
        }
    }
    PUSH_LSTR_ASM("  addi $sp, $sp, 40");
    recover_callee_registers();
}

void save_register(CMM_MIPS_REG reg) {
    if (reg < REG_T0 || reg > REG_S7) {
        cmm_panic("Error: Invalid register number %d.\n", reg);
    }
    push_stack_frame();
    if (reg < REG_S0) {
        PUSH_LSTR_ASM("  sw $t%d, 0($sp)", reg - REG_T0);
    } else {
        PUSH_LSTR_ASM("  sw $s%d, 0($sp)", reg - REG_S0);
    }
}

void recover_register(CMM_MIPS_REG reg) {
    if (reg < REG_T0 || reg > REG_S7) {
        cmm_panic("Error: Invalid register number %d.\n", reg);
    }
    if (reg < REG_S0) {
        PUSH_LSTR_ASM("  lw $t%d, 0($sp)", reg - REG_T0);
    } else {
        PUSH_LSTR_ASM("  lw $s%d, 0($sp)", reg - REG_S0);
    }
    pop_stack_frame();
}

void push_mov_dst(const char* dst, CMM_MIPS_REG src) {
    if (src < REG_T0 || src > REG_S7) {
        cmm_panic("Error: Invalid register number %d.\n", src);
    }
    if (src < REG_S0) {
        PUSH_LSTR_ASM("  move %s, $t%d", dst, src - REG_T0);
    } else {
        PUSH_LSTR_ASM("  move %s, $s%d", dst, src - REG_S0);
    }
}

void push_mov_src(CMM_MIPS_REG dst, const char* src) {
    if (dst < REG_T0 || dst > REG_S7) {
        cmm_panic("Error: Invalid register number %d.\n", dst);
    }
    if (dst < REG_S0) {
        PUSH_LSTR_ASM("  move $t%d, %s", dst - REG_T0, src);
    } else {
        PUSH_LSTR_ASM("  move $s%d, %s", dst - REG_S0, src);
    }
}

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

CMM_MIPS_REG find_first_available_register(int id) {
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

CMM_MIPS_REG find_current_register_for(int id) {
    for (int i = 0; i < 10; i++) {
        if (used_regs.t[i] == id) { return i + REG_T0; }   // t0~t9 对应
    }
    for (int i = 0; i < 8; i++) {
        if (used_regs.s[i] == id) { return i + REG_S0; }   // s0~s7 对应 $s0~$s7
    }
    return REG_ERR;
}

void free_register(CMM_MIPS_REG reg) {
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

CMM_MIPS_REG allocate_reg_for(CMM_IR_VAR var) {
    int id    = var.data.id;
    int poped = mark_var_visited(id);
    if (poped == id) {
        // still in lru
        return find_current_register_for(id);
    }
    CMM_MIPS_REG ret = find_current_register_for(poped);
    // todo
    return ret;
}

void gen_code_from_ir(LArrayOfIR_CODE* irs) {
    size_t irs_len = irs->size;

    for (size_t idx = 0; idx < irs_len; idx++) {
        IR_CODE ir = irs->data[idx];

        // 处理每条 IR 指令
        switch (ir.kind) {
            case IR_LABEL: {
                PUSH_LSTR_ASM("label_%d:", ir.data.label.id);
                break;
            }
            case IR_FUNCTION: {
                PUSH_LSTR_ASM("%s:", ir.data.func_name);
                break;
            }
            case IR_WRITE: {
                CMM_MIPS_REG reg = allocate_reg_for(ir.data.var);
                push_mov_dst("$a0", reg);
                save_function_call_register();
                PUSH_LSTR_ASM("  jal write");
                recover_function_call_register();
                break;
            }
            case IR_READ: {
                CMM_MIPS_REG reg = allocate_reg_for(ir.data.var);
                save_function_call_register();
                PUSH_LSTR_ASM("  jal read");
                recover_function_call_register();
                push_mov_src(reg, "$v0");
                break;
            }
            case IR_CALL: {
                save_caller_registers();
                PUSH_LSTR_ASM("  jal %s", ir.data.call_code.func_name);
                recover_caller_registers();
                CMM_MIPS_REG reg = allocate_reg_for(ir.data.call_code.x);
                push_mov_src(reg, "$v0");
                break;
            }
            case IR_GOTO: {
                PUSH_LSTR_ASM("  j label_%d", ir.data.label.id);
                break;
            }
            case IR_RETURN: {
                CMM_MIPS_REG reg = allocate_reg_for(ir.data.var);
                push_mov_dst("v0", reg);
                PUSH_LSTR_ASM("  jr $ra");
                break;
            }
            case IR_IF: {
                CMM_MIPS_REG reg_x     = allocate_reg_for(ir.data.if_code.x);
                CMM_MIPS_REG reg_y     = allocate_reg_for(ir.data.if_code.y);
                LString      reg_x_str = reg_to_string(reg_x);
                LString      reg_y_str = reg_to_string(reg_y);
                char*        rel       = ir.data.if_code.rel;
                if (strcmp(rel, "==") == 0) {
                    PUSH_LSTR_ASM("  beq %s, %s, label_%d",
                                  reg_x_str->str,
                                  reg_y_str->str,
                                  ir.data.if_code.z.id);
                } else if (strcmp(rel, "!=") == 0) {
                    PUSH_LSTR_ASM("  bne %s, %s, label_%d",
                                  reg_x_str->str,
                                  reg_y_str->str,
                                  ir.data.if_code.z.id);
                } else if (strcmp(rel, "<") == 0) {
                    PUSH_LSTR_ASM("  blt %s, %s, label_%d",
                                  reg_x_str->str,
                                  reg_y_str->str,
                                  ir.data.if_code.z.id);
                } else if (strcmp(rel, "<=") == 0) {
                    PUSH_LSTR_ASM("  ble %s, %s, label_%d",
                                  reg_x_str->str,
                                  reg_y_str->str,
                                  ir.data.if_code.z.id);
                } else if (strcmp(rel, ">") == 0) {
                    PUSH_LSTR_ASM("  bgt %s, %s, label_%d",
                                  reg_x_str->str,
                                  reg_y_str->str,
                                  ir.data.if_code.z.id);
                } else if (strcmp(rel, ">=") == 0) {
                    PUSH_LSTR_ASM("  bge %s, %s, label_%d",
                                  reg_x_str->str,
                                  reg_y_str->str,
                                  ir.data.if_code.z.id);
                } else {
                    cmm_panic("Error: Unknown relational operator '%s'.\n",
                              rel);
                }

                LStringFree(reg_x_str);
                LStringFree(reg_y_str);
                break;
            }
            case IR_2CODE: {
                cmm_debug(COLOR_RED, "not impl");
                break;
            }
            case IR_3CODE: {
                cmm_debug(COLOR_RED, "not impl");
                break;
            }
            case IR_ARG: {
                cmm_debug(COLOR_RED, "not impl");
                break;
            }
            case IR_PARAM: {
                cmm_debug(COLOR_RED, "not impl");
                break;
            }
            case IR_ARG_ADDR: {
                cmm_debug(COLOR_RED, "not impl");
                break;
            }
            case IR_ALLOC: {
                cmm_debug(COLOR_RED, "not impl");
                break;
            }
        }
    }
}

inline LArrayOfLString* get_asm_output() { return GENERATED_ASM; }
void                    free_asm_output() {
    LString* ptr = GENERATED_ASM->data;
    for (size_t i = 0; i < GENERATED_ASM->size; i++) { LStringFree(ptr[i]); }
    FreeLArrayOfLString(GENERATED_ASM);
    GENERATED_ASM = NULL;
}