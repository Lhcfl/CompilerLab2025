#ifndef LINCA_BYYL_IR_H
#define LINCA_BYYL_IR_H

#include <stdio.h>
enum CMM_IR_VAR_KIND {
    IR_VARIABLE,
    IR_IMMEDIATE_INT,
    IR_IMMEDIATE_FLOAT,
};

typedef struct CMM_IR_VAR {
    enum CMM_IR_VAR_KIND kind;
    char*                desc;
    union {
        int   id;
        int   val_int;
        float val_float;
    } data;
} CMM_IR_VAR;

typedef struct CMM_IR_LABEL {
    char* desc;
    int   id;
} CMM_IR_LABEL;

CMM_IR_LABEL ir_new_label(char* desc);
CMM_IR_VAR   ir_new_var(char* desc);
CMM_IR_VAR   ir_new_tmpvar();
CMM_IR_VAR   ir_new_immediate_int(int x);
CMM_IR_VAR   ir_new_immediate_float(float x);

/// FUNCTION f
void gen_ir_function(const char* func_name);
/// LABEL x
void gen_ir_label_start(CMM_IR_LABEL label);
/// x := y
void gen_ir_assign(CMM_IR_VAR dst, CMM_IR_VAR src);
/// x := a + b
void gen_ir_add(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b);
/// x := a - b
void gen_ir_sub(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b);
/// x := a * b
void gen_ir_mul(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b);
/// x := a / b
void gen_ir_div(CMM_IR_VAR dst, CMM_IR_VAR a, CMM_IR_VAR b);
/// x := &y
void gen_ir_get_addr(CMM_IR_VAR dst, CMM_IR_VAR src);
/// x := *y
void gen_ir_dereference(CMM_IR_VAR dst, CMM_IR_VAR src);
/// *x := y
void gen_ir_put_into_addr(CMM_IR_VAR dst, CMM_IR_VAR src);
/// GOTO x
void gen_ir_goto(CMM_IR_LABEL label);
/// IF x [relop] y GOTO z
void gen_ir_if_goto(CMM_IR_VAR   a,
                    CMM_IR_VAR   b,
                    const char*  relop,
                    CMM_IR_LABEL label);
/// RETURN x
void gen_ir_return(CMM_IR_VAR ret);
/// DEC x [size]
void gen_ir_alloc(CMM_IR_VAR x, int size);
/// ARG x
void gen_ir_arg(CMM_IR_VAR x);
/// ARG &x
void gen_ir_arg_addr(CMM_IR_VAR x);
/// x := CALL f
void gen_ir_call(CMM_IR_VAR ret, const char* func_name);
/// PARAM x
void gen_ir_param(CMM_IR_VAR x);

/// READ x
void gen_ir_read(CMM_IR_VAR x);
/// WRITE x
void gen_ir_write(CMM_IR_VAR x);

char* get_ir_output();
#endif