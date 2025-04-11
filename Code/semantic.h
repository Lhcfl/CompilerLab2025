#ifndef LINCA_BYYL_SEMANTICS
#define LINCA_BYYL_SEMANTICS
#include "predefines.h"

enum CMM_SEMANTIC {
    /// 错误的 AST Tree
    CMM_SE_BAD_AST_TREE                  = -2,
    /// 未知错误
    CMM_SE_UNKNOWN                       = -1,
    /// 正确
    CMM_SE_OK                            = 0,
    /// 错误类型1：变量在使用时未经定义。
    CMM_SE_UNDEFINED_VARIABLE            = 1,
    /// 错误类型2：函数在调用时未经定义。
    CMM_SE_UNDEFINED_FUNCTION            = 2,
    /// 错误类型3：变量出现重复定义，或变量与前面定义过的结构体名字重复。
    CMM_SE_DUPLICATE_VARIABLE_DEFINATION = 3,
    /// 错误类型4：函数出现重复定义（即同样的函数名出现了不止一次定义）。
    CMM_SE_DUPLICATE_FUNCTION_DEFINATION = 4,
    /// 错误类型5：赋值号两边的表达式类型不匹配。
    CMM_SE_ASSIGN_TYPE_ERROR             = 5,
    /// 错误类型6：赋值号左边出现一个只有右值的表达式。
    CMM_SE_ASSIGN_TO_RVALUE              = 6,
    /// 错误类型7：操作数类型不匹配或操作数类型与操作符不匹配（例如整型变量与数组变量相加减，或数组（或结构体）变量与数组（或结构体）变量相加减）。
    CMM_SE_OPERAND_TYPE_ERROR            = 7,
    /// 错误类型8：return语句的返回类型与函数定义的返回类型不匹配。
    CMM_SE_RETURN_TYPE_ERROR             = 8,
    /// 错误类型9：函数调用时实参与形参的数目或类型不匹配。
    CMM_SE_ARGS_NOT_MATCH                = 9,
    /// 错误类型10：对非数组型变量使用“[…]”（数组访问）操作符。
    CMM_SE_BAD_ARRAY_ACCESS              = 10,
    /// 错误类型11：对普通变量使用“(…)”或“()”（函数调用）操作符。
    CMM_SE_BAD_FUNCTION_CALL             = 11,
    /// 错误类型12：数组访问操作符“[…]”中出现非整数（例如a[1.5]）。
    CMM_SE_BAD_ARRAY_INDEX               = 12,
    /// 错误类型13：对非结构体型变量使用“.”操作符。
    CMM_SE_BAD_STRUCT_ACCESS             = 13,
    /// 错误类型14：访问结构体中未定义过的域。
    CMM_SE_UNDEFINED_STRUCT_DOMAIN       = 14,
    /// 错误类型15：结构体中域名重复定义（指同一结构体中），或在定义时对域进行初始化（例如struct
    /// A {int a=0;}）。
    CMM_SE_BAD_STRUCT_DOMAIN             = 15,
    /// 错误类型16：结构体的名字与前面定义过的结构体或变量的名字重复。
    CMM_SE_DUPLICATE_STRUCT              = 16,
    /// 错误类型17：直接使用未定义过的结构体来定义变量。
    CMM_SE_UNDEFINED_STRUCT              = 17,
    /// 错误类型18：函数进行了声明，但没有被定义。
    CMM_SE_FUNCTION_DECLARED_NOT_DEFINED = 18,
    /// 错误类型19：函数的多次声明互相冲突（即函数名一致，但返回类型、形参数量或者形参类型不一致），或者声明与定义之间互相冲突。
    CMM_SE_CONFLICT_FUNCTION_DECLARATION = 19,
};

int cmm_semantic_analyze(CMM_AST_NODE* node);

typedef struct CMM_SEMANTIC_ERROR {
    /// 错误类型
    enum CMM_SEMANTIC type;
    /// 错误所在行号
    int               line;
} CMM_SEMANTIC_ERROR;

const char* cmm_semantic_error_to_string(enum CMM_SEMANTIC type);

/// 出错时调用，获得错误数组
CMM_SEMANTIC_ERROR* cmm_get_semantic_errors();

#endif