
#include "syndef.h"

char* cmm_token_tostring(enum CMM_SYNTAX_TOKEN token) {
    switch (token) {
        case CMM_TK_INT: return "INT";
        case CMM_TK_FLOAT: return "FLOAT";
        case CMM_TK_ID: return "ID";
        case CMM_TK_SEMI: return "SEMI";
        case CMM_TK_COMMA: return "COMMA";
        case CMM_TK_ASSIGNOP: return "ASSIGNOP";
        case CMM_TK_RELOP: return "RELOP";
        case CMM_TK_PLUS: return "PLUS";
        case CMM_TK_MINUS: return "MINUS";
        case CMM_TK_STAR: return "STAR";
        case CMM_TK_DIV: return "DIV";
        case CMM_TK_AND: return "AND";
        case CMM_TK_OR: return "OR";
        case CMM_TK_DOT: return "DOT";
        case CMM_TK_NOT: return "NOT";
        case CMM_TK_TYPE: return "TYPE";
        case CMM_TK_LP: return "LP";
        case CMM_TK_RP: return "RP";
        case CMM_TK_LB: return "LB";
        case CMM_TK_RB: return "RB";
        case CMM_TK_LC: return "LC";
        case CMM_TK_RC: return "RC";
        case CMM_TK_STRUCT: return "STRUCT";
        case CMM_TK_RETURN: return "RETURN";
        case CMM_TK_IF: return "IF";
        case CMM_TK_ELSE: return "ELSE";
        case CMM_TK_WHILE: return "WHILE";
        case CMM_TK_Program: return "Program";
        case CMM_TK_ExtDefList: return "ExtDefList";
        case CMM_TK_ExtDef: return "ExtDef";
        case CMM_TK_Specifier: return "Specifier";
        case CMM_TK_ExtDecList: return "ExtDecList";
        case CMM_TK_FunDec: return "FunDec";
        case CMM_TK_CompSt: return "CompSt";
        case CMM_TK_VarDec: return "VarDec";
        case CMM_TK_StructSpecifier: return "StructSpecifier";
        case CMM_TK_OptTag: return "OptTag";
        case CMM_TK_DefList: return "DefList";
        case CMM_TK_Tag: return "Tag";
        case CMM_TK_VarList: return "VarList";
        case CMM_TK_ParamDec: return "ParamDec";
        case CMM_TK_StmtList: return "StmtList";
        case CMM_TK_Stmt: return "Stmt";
        case CMM_TK_Exp: return "Exp";
        case CMM_TK_Def: return "Def";
        case CMM_TK_DecList: return "DecList";
        case CMM_TK_Dec: return "Dec";
        case CMM_TK_Args: return "Args";
    }
}
