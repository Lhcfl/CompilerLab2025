#include "ir.h"
#include "llib.h"
#include "predefines.h"
#include "stddef.h"
#include "stdio.h"
#include "string.h"

#define IR_VAR_BUFFER_SIZE 50
#define IR_LABEL_BUFFER_SIZE 50

LArrayOfIR_CODE* GENERATED_IR = NULL;
char             buffer_ptr[512];

#define PRINT_IR(...)                              \
    {                                              \
        sprintf(buffer_ptr, __VA_ARGS__);          \
        LString str = MakeLStringWith(buffer_ptr); \
        return str;                                \
    }

#define gen_ir_var_name(var)                                             \
    char name_##var[IR_VAR_BUFFER_SIZE];                                 \
    switch (var.kind) {                                                  \
        case IR_VARIABLE: {                                              \
            if (var.data.id > 0) {                                       \
                sprintf(name_##var, "var_%s_%d", var.desc, var.data.id); \
            } else {                                                     \
                sprintf(name_##var, "tmp_%d", -var.data.id);             \
            }                                                            \
            break;                                                       \
        }                                                                \
        case IR_IMMEDIATE_INT: {                                         \
            sprintf(name_##var, "#%d", var.data.val_int);                \
            break;                                                       \
        }                                                                \
        case IR_IMMEDIATE_FLOAT: {                                       \
            sprintf(name_##var, "#%f", var.data.val_float);              \
            break;                                                       \
        }                                                                \
    }

#define gen_ir_label_name(label)             \
    char name_##label[IR_LABEL_BUFFER_SIZE]; \
    sprintf(name_##label, "label_%s_%d", label.desc, label.id);

CMM_IR_LABEL ir_new_label(char* desc) {
    static int   label_count = 0;
    CMM_IR_LABEL label;
    label.id   = label_count++;
    label.desc = desc;
    return label;
}

CMM_IR_VAR ir_new_var(char* desc) {
    static int var_count = 0;
    var_count++;
    return (CMM_IR_VAR){
        .kind    = IR_VARIABLE,
        .data.id = var_count,
        .desc    = desc,
    };
}

CMM_IR_VAR ir_new_tmpvar() {
    static int var_count = 1;
    var_count--;
    return (CMM_IR_VAR){
        .kind    = IR_VARIABLE,
        .data.id = var_count,
        .desc    = NULL,
    };
}

CMM_IR_VAR ir_new_immediate_int(int x) {
    return (CMM_IR_VAR){
        .kind         = IR_IMMEDIATE_INT,
        .data.val_int = x,
    };
}

CMM_IR_VAR ir_new_immediate_float(float x) {
    return (CMM_IR_VAR){
        .kind           = IR_IMMEDIATE_FLOAT,
        .data.val_float = x,
    };
}

/// FUNCTION f
void gen_ir_function(const char* func_name) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind           = IR_FUNCTION,
                            .data.func_name = (char*)func_name,
                        });
}

/// LABEL x
void gen_ir_label_start(CMM_IR_LABEL label) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind       = IR_LABEL,
                            .data.label = label,
                        });
}

/// x := y
void gen_ir_assign(CMM_IR_VAR dst, CMM_IR_VAR src) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind = IR_2CODE,
                            .data.two_addr =
                                {
                                    .x  = dst,
                                    .op = IR_ASSIGN,
                                    .z  = src,
                                },
                        });
}

/// x := a + b
void gen_ir_add(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind = IR_3CODE,
                            .data.three_addr =
                                {
                                    .x  = dst,
                                    .y  = a,
                                    .op = IR_ADD,
                                    .z  = b,
                                },
                        });
}
/// x := a - b
void gen_ir_sub(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind = IR_3CODE,
                            .data.three_addr =
                                {
                                    .x  = dst,
                                    .y  = a,
                                    .op = IR_SUB,
                                    .z  = b,
                                },
                        });
}
/// x := a * b
void gen_ir_mul(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind = IR_3CODE,
                            .data.three_addr =
                                {
                                    .x  = dst,
                                    .y  = a,
                                    .op = IR_MUL,
                                    .z  = b,
                                },
                        });
}
/// x := a / b
void gen_ir_div(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind = IR_3CODE,
                            .data.three_addr =
                                {
                                    .x  = dst,
                                    .y  = a,
                                    .op = IR_DIV,
                                    .z  = b,
                                },
                        });
}
/// x := &y
void gen_ir_get_addr(CMM_IR_VAR dst, CMM_IR_VAR src) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind = IR_2CODE,
                            .data.two_addr =
                                {
                                    .x  = dst,
                                    .op = IR_GETADDR,
                                    .z  = src,
                                },
                        });
}
/// x := *y
void gen_ir_dereference(CMM_IR_VAR dst, CMM_IR_VAR src) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind = IR_2CODE,
                            .data.two_addr =
                                {
                                    .x  = dst,
                                    .op = IR_DEREF,
                                    .z  = src,
                                },
                        });
}
/// *x := y
void gen_ir_put_into_addr(CMM_IR_VAR dst, CMM_IR_VAR src) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind = IR_2CODE,
                            .data.two_addr =
                                {
                                    .x  = dst,
                                    .op = IR_PUTADDR,
                                    .z  = src,
                                },
                        });
}
/// GOTO x
void gen_ir_goto(CMM_IR_LABEL label) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind       = IR_GOTO,
                            .data.label = label,
                        });
}
/// IF x [relop] y GOTO z
void gen_ir_if_goto(CMM_IR_VAR   a,
                    CMM_IR_VAR   b,
                    const char*  relop,
                    CMM_IR_LABEL label) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind = IR_IF,
                            .data.if_code =
                                {
                                    .x   = a,
                                    .y   = b,
                                    .rel = (char*)relop,
                                    .z   = label,
                                },
                        });
}
/// RETURN x
void gen_ir_return(CMM_IR_VAR ret) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind     = IR_RETURN,
                            .data.var = ret,
                        });
}
/// DEC x [size]
void gen_ir_alloc(CMM_IR_VAR x, int size) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind = IR_ALLOC,
                            .data.alloc_code =
                                {
                                    .size = size,
                                    .x    = x,
                                },
                        });
}
/// ARG x
void gen_ir_arg(CMM_IR_VAR x) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind     = IR_ARG,
                            .data.var = x,
                        });
}
/// ARG &x
void gen_ir_arg_addr(CMM_IR_VAR x) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind     = IR_ARG_ADDR,
                            .data.var = x,
                        });
}
/// x := CALL f
void gen_ir_call(CMM_IR_VAR ret, const char* func_name) {
    PushLArrayOfIR_CODE(
        GENERATED_IR,
        (IR_CODE){
            .kind           = IR_CALL,
            .data.call_code = {.x = ret, .func_name = (char*)func_name},
        });
}

/// PARAM x
void gen_ir_param(CMM_IR_VAR x) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind     = IR_PARAM,
                            .data.var = x,
                        });
}

/// READ x
void gen_ir_read(CMM_IR_VAR x) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind     = IR_READ,
                            .data.var = x,
                        });
}

/// WRITE x
void gen_ir_write(CMM_IR_VAR x) {
    PushLArrayOfIR_CODE(GENERATED_IR,
                        (IR_CODE){
                            .kind     = IR_WRITE,
                            .data.var = x,
                        });
}

/// FUNCTION f
LString to_string_ir_function(const char* func_name) {
    static int func_count = 0;
    if (func_count > 0) { PRINT_IR(" "); }
    func_count++;
    PRINT_IR("FUNCTION %s :", func_name);
}

/// LABEL x
LString to_string_ir_label_start(CMM_IR_LABEL label) {
    gen_ir_label_name(label);
    PRINT_IR("LABEL %s :", name_label);
}

/// x := y
LString to_string_ir_assign(CMM_IR_VAR dst, CMM_IR_VAR src) {
    gen_ir_var_name(dst);
    gen_ir_var_name(src);
    PRINT_IR("%s := %s", name_dst, name_src);
}

/// x := a + b
LString to_string_ir_add(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {
    gen_ir_var_name(dst);
    gen_ir_var_name(a);
    gen_ir_var_name(b);
    PRINT_IR("%s := %s + %s", name_dst, name_a, name_b);
}
/// x := a - b
LString to_string_ir_sub(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {
    gen_ir_var_name(dst);
    gen_ir_var_name(a);
    gen_ir_var_name(b);
    PRINT_IR("%s := %s - %s", name_dst, name_a, name_b);
}
/// x := a * b
LString to_string_ir_mul(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {
    gen_ir_var_name(dst);
    gen_ir_var_name(a);
    gen_ir_var_name(b);
    PRINT_IR("%s := %s * %s", name_dst, name_a, name_b);
}
/// x := a / b
LString to_string_ir_div(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {
    gen_ir_var_name(dst);
    gen_ir_var_name(a);
    gen_ir_var_name(b);
    PRINT_IR("%s := %s / %s", name_dst, name_a, name_b);
}
/// x := &y
LString to_string_ir_get_addr(CMM_IR_VAR dst, CMM_IR_VAR src) {
    gen_ir_var_name(dst);
    gen_ir_var_name(src);
    PRINT_IR("%s := &%s", name_dst, name_src);
}
/// x := *y
LString to_string_ir_dereference(CMM_IR_VAR dst, CMM_IR_VAR src) {
    gen_ir_var_name(dst);
    gen_ir_var_name(src);
    PRINT_IR("%s := *%s", name_dst, name_src);
}
/// *x := y
LString to_string_ir_put_into_addr(CMM_IR_VAR dst, CMM_IR_VAR src) {
    gen_ir_var_name(dst);
    gen_ir_var_name(src);
    PRINT_IR("*%s := %s", name_dst, name_src);
}
/// GOTO x
LString to_string_ir_goto(CMM_IR_LABEL label) {
    gen_ir_label_name(label);
    PRINT_IR("GOTO %s", name_label);
}
/// IF x [relop] y GOTO z
LString to_string_ir_if_goto(CMM_IR_VAR   a,
                             CMM_IR_VAR   b,
                             const char*  relop,
                             CMM_IR_LABEL label) {
    gen_ir_var_name(a);
    gen_ir_var_name(b);
    gen_ir_label_name(label);
    PRINT_IR("IF %s %s %s GOTO %s", name_a, relop, name_b, name_label);
}
/// RETURN x
LString to_string_ir_return(CMM_IR_VAR ret) {
    gen_ir_var_name(ret);
    PRINT_IR("RETURN %s", name_ret);
}
/// DEC x [size]
LString to_string_ir_alloc(CMM_IR_VAR x, int size) {
    gen_ir_var_name(x);
    PRINT_IR("DEC %s %d", name_x, size * 4);
}
/// ARG x
LString to_string_ir_arg(CMM_IR_VAR x) {
    gen_ir_var_name(x);
    PRINT_IR("ARG %s", name_x);
}
/// ARG &x
LString to_string_ir_arg_addr(CMM_IR_VAR x) {
    gen_ir_var_name(x);
    PRINT_IR("ARG &%s", name_x);
}
/// x := CALL f
LString to_string_ir_call(CMM_IR_VAR ret, const char* func_name) {
    gen_ir_var_name(ret);
    PRINT_IR("%s := CALL %s", name_ret, func_name);
}

/// PARAM x
LString to_string_ir_param(CMM_IR_VAR x) {
    gen_ir_var_name(x);
    PRINT_IR("PARAM %s", name_x);
}

/// READ x
LString to_string_ir_read(CMM_IR_VAR x) {
    gen_ir_var_name(x);
    PRINT_IR("READ %s", name_x);
}

/// WRITE x
LString to_string_ir_write(CMM_IR_VAR x) {
    gen_ir_var_name(x);
    PRINT_IR("WRITE %s", name_x);
}

LString ir_code_to_string(IR_CODE code) {
    switch (code.kind) {
        case IR_FUNCTION: return to_string_ir_function(code.data.func_name);
        case IR_LABEL: return to_string_ir_label_start(code.data.label);
        case IR_2CODE:
            switch (code.data.two_addr.op) {
                case IR_ASSIGN:
                    return to_string_ir_assign(code.data.two_addr.x,
                                               code.data.two_addr.z);
                case IR_GETADDR:
                    return to_string_ir_get_addr(code.data.two_addr.x,
                                                 code.data.two_addr.z);
                case IR_DEREF:
                    return to_string_ir_dereference(code.data.two_addr.x,
                                                    code.data.two_addr.z);
                case IR_PUTADDR:
                    return to_string_ir_put_into_addr(code.data.two_addr.x,
                                                      code.data.two_addr.z);
            }
            break;
        case IR_3CODE:
            switch (code.data.three_addr.op) {
                case IR_ADD:
                    return to_string_ir_add(code.data.three_addr.x,
                                            code.data.three_addr.y,
                                            code.data.three_addr.z);
                case IR_SUB:
                    return to_string_ir_sub(code.data.three_addr.x,
                                            code.data.three_addr.y,
                                            code.data.three_addr.z);
                case IR_MUL:
                    return to_string_ir_mul(code.data.three_addr.x,
                                            code.data.three_addr.y,
                                            code.data.three_addr.z);
                case IR_DIV:
                    return to_string_ir_div(code.data.three_addr.x,
                                            code.data.three_addr.y,
                                            code.data.three_addr.z);
            }
            break;
        case IR_GOTO: return to_string_ir_goto(code.data.label);
        case IR_IF:
            return to_string_ir_if_goto(code.data.if_code.x,
                                        code.data.if_code.y,
                                        code.data.if_code.rel,
                                        code.data.if_code.z);
        case IR_RETURN: return to_string_ir_return(code.data.var);
        case IR_ALLOC:
            return to_string_ir_alloc(code.data.alloc_code.x,
                                      code.data.alloc_code.size);
        case IR_CALL:
            return to_string_ir_call(code.data.call_code.x,
                                     code.data.call_code.func_name);
        case IR_ARG: return to_string_ir_arg(code.data.var);
        case IR_ARG_ADDR: return to_string_ir_arg_addr(code.data.var);
        case IR_READ: return to_string_ir_read(code.data.var);
        case IR_WRITE: return to_string_ir_write(code.data.var);
        case IR_PARAM: return to_string_ir_param(code.data.var);
    }
    cmm_panic("unreachable: ir_code_to_string");
}

LArrayOfIR_CODE* get_ir_output() { return GENERATED_IR; }
void             initialize_ir() { GENERATED_IR = NewLArrayOfIR_CODE(); }
void             free_all_ir() {
    FreeLArrayOfIR_CODE(GENERATED_IR);
    GENERATED_IR = NULL;
}