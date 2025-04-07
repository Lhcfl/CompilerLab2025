#include "semantic.h"
#include "avl.h"
#include "predefines.h"
#include "syndef.h"

#define ANALYZE_EXPECT_OK(x)            \
    {                                   \
        if ((x) != CMM_SE_OK) return x; \
    }

enum CMM_SEMANTIC analyze_program(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_ext_def_list(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_ext_def(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_specifier(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_ext_dec_list(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_fun_dec(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_comp_st(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_var_dec(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_struct_specifier(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_opt_tag(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_def_list(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_tag(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_var_list(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_param_dec(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_stmt_list(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_stmt(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_exp(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_def(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_dec_list(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_dec(CMM_AST_NODE* node);
enum CMM_SEMANTIC analyze_args(CMM_AST_NODE* node);

AVLTree analyze_context;

CMM_SEM_TYPE make_type_primitive(char* name) {
    CMM_SEM_TYPE ret;
    ret.kind  = CMM_PRIMITIVE_TYPE;
    ret.name  = name;
    ret.bind  = NULL;
    ret.inner = NULL;
    ret.next  = NULL;
    return ret;
}

int cmm_semantic_analyze(CMM_AST_NODE* node) {
    enum CMM_SEMANTIC x = analyze_program(node);
    if (x != CMM_SE_OK) { return 1; }
    return 0;
}

/// Program: ExtDefList;
enum CMM_SEMANTIC analyze_program(CMM_AST_NODE* node) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Program) { return CMM_SE_BAD_AST_TREE; }

    if (node->len != 1) { return CMM_SE_BAD_AST_TREE; }
    return analyze_ext_def_list(node->nodes);
}

/// ExtDefList: /* empty */ | ExtDef ExtDefList
enum CMM_SEMANTIC analyze_ext_def_list(CMM_AST_NODE* node) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_ExtDefList) { return CMM_SE_BAD_AST_TREE; }

    if (node->len == 0) { return CMM_SE_OK; }

    if (node->len == 2) {
        analyze_ext_def(node->nodes + 0);
        // TODO
        return analyze_ext_def_list(node->nodes + 1);
    }

    return CMM_SE_BAD_AST_TREE;
}

/// ExtDef: Specifier ExtDecList SEMI
/// | Specifier SEMI
/// | Specifier FunDec CompSt
/// ;
enum CMM_SEMANTIC analyze_ext_def(CMM_AST_NODE* node) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_ExtDef) { return CMM_SE_BAD_AST_TREE; }

    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* decl      = node->nodes + 1;

    /// 无论这里有没有出错，我们让它继续往下走
    /// 错误的类型会被置为 error 这个特殊的原型类
    analyze_specifier(specifier);

    if (decl->kind == CMM_TK_ExtDecList) {
        // TODO
        analyze_ext_dec_list(decl);
    } else if (decl->kind == CMM_TK_FunDec) {
        // TODO
        analyze_fun_dec(decl);
    } else if (decl->kind == CMM_TK_SEMI) {
        /// 在这里声明了一个 Type
        /// TODO
        return CMM_SE_OK;
    }

    return CMM_SE_BAD_AST_TREE;
}

/// ExtDecList: VarDec | VarDec COMMA ExtDecList
enum CMM_SEMANTIC analyze_ext_dec_list(CMM_AST_NODE* node) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_ExtDecList) { return CMM_SE_BAD_AST_TREE; }

    enum CMM_SEMANTIC vardec = analyze_var_dec(node->nodes + 0);
    if (node->len == 1) {
        return CMM_SE_OK;
    } else if (node->len == 3) {
        return analyze_ext_dec_list(node->nodes + 2);
    }

    return CMM_SE_BAD_AST_TREE;
}


/// Specifier: TYPE | StructSpecifier
enum CMM_SEMANTIC analyze_specifier(CMM_AST_NODE* node) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Specifier) { return CMM_SE_BAD_AST_TREE; }

    if (node->len != 1) { return CMM_SE_BAD_AST_TREE; }

    CMM_AST_NODE* inner = node->nodes;
    node->context.kind  = CMM_AST_KIND_TYPE;

    if (inner->kind == CMM_TK_TYPE) {
        node->context.data.type = make_type_primitive(node->data.val_type);
        return CMM_SE_OK;
    } else if (inner->kind == CMM_TK_StructSpecifier) {
        enum CMM_SEMANTIC res = analyze_struct_specifier(node->nodes + 0);
        if (res == CMM_SE_OK) {
            node->context.data.type = inner->context.data.type;
        } else {
            node->context.data.type = make_type_primitive("error");
        }
    }

    return CMM_SE_BAD_AST_TREE;
}

/// TODO
enum CMM_SEMANTIC analyze_struct_specifier(CMM_AST_NODE* node);

/// Tag: empty | ID
enum CMM_SEMANTIC analyze_opt_tag(CMM_AST_NODE* node) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_OptTag) { return CMM_SE_BAD_AST_TREE; }
    if (node->len == 0) { return CMM_SE_OK; }

    if (node->len != 1) { return CMM_SE_BAD_AST_TREE; }
    CMM_AST_NODE* tag = node->nodes + 0;
    if (tag->kind != CMM_TK_Tag) { return CMM_SE_BAD_AST_TREE; }

    node->context.kind       = CMM_AST_KIND_VAR;
    node->context.data.ident = tag->data.val_ident;
    return CMM_SE_OK;
};


/// Tag: ID
enum CMM_SEMANTIC analyze_tag(CMM_AST_NODE* node) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Tag) { return CMM_SE_BAD_AST_TREE; }
    if (node->len != 1) { return CMM_SE_BAD_AST_TREE; }

    CMM_AST_NODE* tag = node->nodes + 0;
    if (tag->kind != CMM_TK_ID) { return CMM_SE_BAD_AST_TREE; }

    node->context.kind       = CMM_AST_KIND_VAR;
    node->context.data.ident = tag->data.val_ident;
    return CMM_SE_OK;
}

/// VarDec: ID | VarDec LB INT RB
/// TODO
enum CMM_SEMANTIC analyze_var_dec(CMM_AST_NODE* node);

/// FunDec: ID LP VarList RP | ID LP RP
/// TODO
enum CMM_SEMANTIC analyze_fun_dec(CMM_AST_NODE* node);

/// VarList: ParamDec COMMA VarList | ParamDec
/// TODO
enum CMM_SEMANTIC analyze_var_list(CMM_AST_NODE* node);

/// ParamDec: Specifier VarDec
/// TODO
enum CMM_SEMANTIC analyze_param_dec(CMM_AST_NODE* node);

/// CompSt: LC DefList StmtList RC
/// TODO
enum CMM_SEMANTIC analyze_comp_st(CMM_AST_NODE* node);

/// StmtList: /* empty */ | Stmt StmtList
/// TODO
enum CMM_SEMANTIC analyze_stmt_list(CMM_AST_NODE* node);

// Stmt: Exp SEMI
//     | CompSt
//     | RETURN Exp SEMI
//     | IF LP Exp RP Stmt
//     | IF LP Exp RP Stmt ELSE Stmt
//     | WHILE LP Exp RP Stmt
/// TODO
enum CMM_SEMANTIC analyze_stmt(CMM_AST_NODE* node);

/// DefList: /* empty */ | Def DefList
/// TODO
enum CMM_SEMANTIC analyze_def_list(CMM_AST_NODE* node);

/// Def: Specifier DecList SEMI
/// TODO
enum CMM_SEMANTIC analyze_def(CMM_AST_NODE* node);

/// DecList: Dec | Dec COMMA DecList
/// TODO
enum CMM_SEMANTIC analyze_dec_list(CMM_AST_NODE* node);



/// Dec: VarDec | VarDec ASSIGNOP Exp
/// TODO
enum CMM_SEMANTIC analyze_dec(CMM_AST_NODE* node);


// Exp: Exp ASSIGNOP Exp
//     | Exp OR Exp
//     | Exp AND Exp
//     | Exp RELOP Exp
//     | Exp PLUS Exp
//     | Exp MINUS Exp
//     | Exp STAR Exp
//     | Exp DIV Exp
//     | MINUS Exp
//     | NOT Exp
//     | LP Exp RP
//     | ID LP Args RP
//     | ID LP RP
//     | Exp LB Exp RB
//     | Exp DOT ID
//     | ID
//     | INT
//     | FLOAT
/// TODO
enum CMM_SEMANTIC analyze_exp(CMM_AST_NODE* node);


// Args: Exp COMMA Args
//     | Exp
/// TODO
enum CMM_SEMANTIC analyze_args(CMM_AST_NODE* node);
