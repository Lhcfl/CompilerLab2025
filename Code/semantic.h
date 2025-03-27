#ifndef LINCA_BYYL_SEMANTICS
#define LINCA_BYYL_SEMANTICS
#include "predefines.h"

enum CMM_SEMANTIC_ERROR {
    CMM_SE_UNDEFINED_VARIABLE            = 1,
    CMM_SE_UNDEFINED_FUNCTION            = 2,
    CMM_SE_DUPLICATE_VARIABLE_DEFINATION = 3,
    CMM_SE_DUPLICATE_FUNCTION_DEFINATION = 4,
    CMM_SE_ASSIGN_TYPE_ERROR             = 5,
    CMM_SE_ASSIGN_TO_RVALUE              = 6,
    CMM_SE_OPERAND_TYPE_ERROR            = 7,
    CMM_SE_RETURN_TYPE_ERROR             = 8,
    CMM_SE_ARGS_NOT_MATCH                = 9,
    CMM_SE_BAD_ARRAY_ACCESS              = 10,
    CMM_SE_BAD_FUNCTION_CALL             = 11,
    CMM_SE_BAD_ARRAY_INDEX               = 12,
    CMM_SE_BAD_STRUCT_ACCESS             = 13,
    CMM_SE_UNDEFINED_STRUCT_DOMAIN       = 14,
    CMM_SE_BAD_STRUCT_DOMAIN             = 15,
    CMM_SE_DUPLICATE_STRUCT              = 16,
    CMM_SE_UNDEFINED_STRUCT              = 17,
    CMM_SE_FUNCTION_DECLARED_NOT_DEFINED = 18,
    CMM_SE_CONFLICT_FUNCTION_DECLARAtiON = 19,
};

int cmm_semantic_analyze(CMM_AST_NODE* node);

#endif