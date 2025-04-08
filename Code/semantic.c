#include "semantic.h"
#include "hashmap.h"
#include "predefines.h"
#include "syndef.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// #region Definations

#define ANALYZE_EXPECT_OK(x)            \
    {                                   \
        if ((x) != CMM_SE_OK) return x; \
    }

typedef struct SemanticContext {
    char* name;
    void* data;
} SemanticContext;

typedef struct StringList {
    char*              name;
    struct StringList* back;
} StringList;

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

// #endregion

// #region Global States

/// 错误列表
CMM_SEMANTIC_ERROR semantic_errors[256];
size_t             semantic_errors_count = 0;
/// 一个哈希表，用来存放semantic的context
struct hashmap*    semantic_context      = NULL;
/// 一个链表，用来存放当前scope的名字。越是顶层越在后方
StringList*        semantic_scope        = NULL;
/// 一个链表，用来存放当前scope内产生的definations等等
StringList*        semantic_definitions  = NULL;

// #endregion


// Functions

void free_semantic_ctx(void* data) {
    if (data == NULL) return;
    const SemanticContext* ctx = data;
    free(ctx->name);
    free(ctx->data);
}

int semantic_ctx_compare(const void* a, const void* b, void* udata) {
    const SemanticContext* ua = a;
    const SemanticContext* ub = b;
    return strcmp(ua->name, ub->name);
}

uint64_t senamtic_ctx_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const SemanticContext* ctx = item;
    return hashmap_sip(ctx->name, strlen(ctx->name), seed0, seed1);
}


/// 进入一个新的语义分析作用域
StringList* enter_semantic_scope(char* name) {
    StringList* scope = (StringList*)malloc(sizeof(StringList));
    scope->name       = cmm_clone_string(name);
    scope->back       = semantic_scope;
    semantic_scope    = scope;
    return scope;
}

/// 释放当前 StringList 节点，返回它的前驱（或者说后驱）
StringList* free_string_list_node(StringList* node) {
    if (node == NULL) { return NULL; }
    StringList* ret = node->back;
    free(node->name);
    free(node);
    return ret;
}

/// 退出当前的语义分析作用域
void exit_semantic_scope() {
    StringList* scope = semantic_scope;
    semantic_scope    = semantic_scope->back;

    // 释放当前作用域的所有定义
    while ((semantic_definitions) != NULL) {
        SemanticContext* deleted = (SemanticContext*)hashmap_delete(
            semantic_context, &(SemanticContext){.name = semantic_definitions->name});

        if (deleted != NULL) { free_semantic_ctx(deleted); }

        semantic_definitions = free_string_list_node(semantic_definitions);
    }

    free(scope->name);
    free(semantic_scope);
}

/// 在当前作用域下产生一个新的定义
void push_defination(char* name) {
    StringList* def      = (StringList*)malloc(sizeof(StringList));
    def->name            = cmm_concat_string(3, semantic_scope->name, ":def:", name);
    def->back            = semantic_definitions;
    semantic_definitions = def;
}

void record_error(int lineno, enum CMM_SEMANTIC error) {
    // too many errors
    if (semantic_errors_count >= 256) { return; }
    semantic_errors[semantic_errors_count].line = lineno;
    semantic_errors[semantic_errors_count].type = error;
    semantic_errors_count++;
}

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
    // prepare
    semantic_context = hashmap_new(sizeof(SemanticContext),
                                   65535,
                                   0,
                                   0,
                                   senamtic_ctx_hash,
                                   semantic_ctx_compare,
                                   free_semantic_ctx,
                                   NULL);
    enter_semantic_scope("root");

    /// analyze
    enum CMM_SEMANTIC x = analyze_program(node);

    // clean
    hashmap_free(semantic_context);
    exit_semantic_scope();

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

/// StructSpecifier: STRUCT OptTag LC DefList RC | STRUCT Tag
enum CMM_SEMANTIC analyze_struct_specifier(CMM_AST_NODE* node) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_StructSpecifier) { return CMM_SE_BAD_AST_TREE; }

    CMM_AST_NODE* tag = node->nodes + 1;

    if (node->len == 2) {
        ANALYZE_EXPECT_OK(analyze_tag(node->nodes + 0));
        // Struct 不可以声明


    } else if (node->len == 4) {
        // OptTag LC DefList RC
        CMM_AST_NODE* opttag  = node->nodes + 1;
        CMM_AST_NODE* deflist = node->nodes + 2;

        enum CMM_SEMANTIC res = analyze_opt_tag(opttag);
        if (res != CMM_SE_OK) { return res; }

        // TODO
        analyze_def_list(deflist);
    }

    return CMM_SE_BAD_AST_TREE;
}

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
