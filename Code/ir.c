#include "ir.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define IR_VAR_BUFFER_SIZE 50
#define IR_LABEL_BUFFER_SIZE 50

char  buffer[30000];
char* buffer_ptr = buffer;

#define PRINT_IR(...)                     \
    {                                     \
        sprintf(buffer_ptr, __VA_ARGS__); \
        buffer_ptr += strlen(buffer_ptr); \
        printf(__VA_ARGS__);              \
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
    static int func_count = 0;
    if (func_count > 0) { PRINT_IR("\n"); }
    func_count++;
    PRINT_IR("FUNCTION %s :\n", func_name);
}

/// LABEL x
void gen_ir_label_start(CMM_IR_LABEL label) {
    gen_ir_label_name(label);
    PRINT_IR("LABEL %s :\n", name_label);
}

/// x := y
void gen_ir_assign(CMM_IR_VAR dst, CMM_IR_VAR src) {
    gen_ir_var_name(dst);
    gen_ir_var_name(src);
    PRINT_IR("%s := %s\n", name_dst, name_src);
}

/// x := a + b
void gen_ir_add(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {

    gen_ir_var_name(dst);
    gen_ir_var_name(a);
    gen_ir_var_name(b);
    PRINT_IR("%s := %s + %s\n", name_dst, name_a, name_b);
}
/// x := a - b
void gen_ir_sub(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {
    gen_ir_var_name(dst);
    gen_ir_var_name(a);
    gen_ir_var_name(b);
    PRINT_IR("%s := %s - %s\n", name_dst, name_a, name_b);
}
/// x := a * b
void gen_ir_mul(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {
    gen_ir_var_name(dst);
    gen_ir_var_name(a);
    gen_ir_var_name(b);
    PRINT_IR("%s := %s * %s\n", name_dst, name_a, name_b);
}
/// x := a / b
void gen_ir_div(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b) {
    gen_ir_var_name(dst);
    gen_ir_var_name(a);
    gen_ir_var_name(b);
    PRINT_IR("%s := %s / %s\n", name_dst, name_a, name_b);
}
/// x := &y
void gen_ir_get_addr(CMM_IR_VAR dst, CMM_IR_VAR src) {
    gen_ir_var_name(dst);
    gen_ir_var_name(src);
    PRINT_IR("%s := &%s\n", name_dst, name_src);
}
/// x := *y
void gen_ir_dereference(CMM_IR_VAR dst, CMM_IR_VAR src) {
    gen_ir_var_name(dst);
    gen_ir_var_name(src);
    PRINT_IR("%s := *%s\n", name_dst, name_src);
}
/// *x := y
void gen_ir_put_into_addr(CMM_IR_VAR dst, CMM_IR_VAR src) {
    gen_ir_var_name(dst);
    gen_ir_var_name(src);
    PRINT_IR("*%s := %s\n", name_dst, name_src);
}
/// GOTO x
void gen_ir_goto(CMM_IR_LABEL label) {
    gen_ir_label_name(label);
    PRINT_IR("GOTO %s\n", name_label);
}
/// IF x [relop] y GOTO z
void gen_ir_if_goto(CMM_IR_VAR   a,
                    CMM_IR_VAR   b,
                    const char*  relop,
                    CMM_IR_LABEL label) {
    gen_ir_var_name(a);
    gen_ir_var_name(b);
    gen_ir_label_name(label);
    PRINT_IR("IF %s %s %s GOTO %s\n", name_a, relop, name_b, name_label);
}
/// RETURN x
void gen_ir_return(CMM_IR_VAR ret) {
    gen_ir_var_name(ret);
    PRINT_IR("RETURN %s\n", name_ret);
}
/// DEC x [size]
void gen_ir_alloc(CMM_IR_VAR x, int size) {
    gen_ir_var_name(x);
    PRINT_IR("DEC %s %d\n", name_x, size * 4);
}
/// ARG x
void gen_ir_arg(CMM_IR_VAR x) {
    gen_ir_var_name(x);
    PRINT_IR("ARG %s\n", name_x);
}
/// x := CALL f
void gen_ir_call(CMM_IR_VAR ret, const char* func_name) {
    gen_ir_var_name(ret);
    PRINT_IR("%s := CALL %s\n", name_ret, func_name);
}
/// PARAM x
void gen_ir_param(CMM_IR_VAR x) {
    gen_ir_var_name(x);
    PRINT_IR("PARAM %s\n", name_x);
}

/// READ x
void gen_ir_read(CMM_IR_VAR x) {
    gen_ir_var_name(x);
    PRINT_IR("READ %s\n", name_x);
}
/// WRITE x
void gen_ir_write(CMM_IR_VAR x) {
    gen_ir_var_name(x);
    PRINT_IR("WRITE %s\n", name_x);
}

char* get_ir_output() { return buffer; }