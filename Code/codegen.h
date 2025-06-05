//! Lab4. Target Code Generation
#ifndef LINCA_BYYL_CODEGEN
#define LINCA_BYYL_CODEGEN

/// 翻译 IR 到目标代码
#include "llib.h"
int cmm_gen_code();

LArrayOfLString* get_asm_output();
void             free_asm_output();
#endif