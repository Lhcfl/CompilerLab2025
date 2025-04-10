#include "semantic.h"
#include "hashmap.h"
#include "predefines.h"
#include "syndef.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#pragma region Definations
#define ANALYZE_EXPECT_OK(x)            \
    {                                   \
        if ((x) != CMM_SE_OK) return x; \
    }

typedef struct SemanticContext {
    char*        name;
    CMM_SEM_TYPE ty;
} SemanticContext;

typedef struct StringList {
    char*              name;
    struct StringList* back;
    union {
        struct StringList* data;
        SemanticContext*   ctx;
    };
} StringList;

struct AnalyCtxProgram {};
struct AnalyCtxExtDefList {};
struct AnalyCtxExtDef {};
struct AnalyCtxSpecifier {};
struct AnalyCtxExtDecList {
    CMM_SEM_TYPE ty;
};
struct AnalyCtxFunDec {};
struct AnalyCtxCompSt {};
struct AnalyCtxVarDec {
    enum {
        INSIDE_A_BLOCK,
        INSIDE_A_STRUCT,
    } where;
    CMM_SEM_TYPE ty;
};
struct AnalyCtxStructSpecifier {};
struct AnalyCtxOptTag {};
struct AnalyCtxDefList {};
struct AnalyCtxTag {};
struct AnalyCtxVarList {};
struct AnalyCtxParamDec {};
struct AnalyCtxStmtList {};
struct AnalyCtxStmt {};
struct AnalyCtxExp {};
struct AnalyCtxDef {};
struct AnalyCtxDecList {};
struct AnalyCtxDec {};
struct AnalyCtxArgs {};

enum CMM_SEMANTIC analyze_program(CMM_AST_NODE* node, struct AnalyCtxProgram args);
enum CMM_SEMANTIC analyze_ext_def_list(CMM_AST_NODE*             node,
                                       struct AnalyCtxExtDefList args);
enum CMM_SEMANTIC analyze_ext_def(CMM_AST_NODE* node, struct AnalyCtxExtDef args);
enum CMM_SEMANTIC analyze_specifier(CMM_AST_NODE* node, struct AnalyCtxSpecifier args);
enum CMM_SEMANTIC analyze_ext_dec_list(CMM_AST_NODE*             node,
                                       struct AnalyCtxExtDecList args);
enum CMM_SEMANTIC analyze_fun_dec(CMM_AST_NODE* node, struct AnalyCtxFunDec args);
enum CMM_SEMANTIC analyze_comp_st(CMM_AST_NODE* node, struct AnalyCtxCompSt args);
enum CMM_SEMANTIC analyze_var_dec(CMM_AST_NODE* node, struct AnalyCtxVarDec args);
enum CMM_SEMANTIC analyze_struct_specifier(CMM_AST_NODE*                  node,
                                           struct AnalyCtxStructSpecifier args);
enum CMM_SEMANTIC analyze_opt_tag(CMM_AST_NODE* node, struct AnalyCtxOptTag args);
enum CMM_SEMANTIC analyze_def_list(CMM_AST_NODE* node, struct AnalyCtxDefList args);
enum CMM_SEMANTIC analyze_tag(CMM_AST_NODE* node, struct AnalyCtxTag args);
enum CMM_SEMANTIC analyze_var_list(CMM_AST_NODE* node, struct AnalyCtxVarList args);
enum CMM_SEMANTIC analyze_param_dec(CMM_AST_NODE* node, struct AnalyCtxParamDec args);
enum CMM_SEMANTIC analyze_stmt_list(CMM_AST_NODE* node, struct AnalyCtxStmtList args);
enum CMM_SEMANTIC analyze_stmt(CMM_AST_NODE* node, struct AnalyCtxStmt args);
enum CMM_SEMANTIC analyze_exp(CMM_AST_NODE* node, struct AnalyCtxExp args);
enum CMM_SEMANTIC analyze_def(CMM_AST_NODE* node, struct AnalyCtxDef args);
enum CMM_SEMANTIC analyze_dec_list(CMM_AST_NODE* node, struct AnalyCtxDecList args);
enum CMM_SEMANTIC analyze_dec(CMM_AST_NODE* node, struct AnalyCtxDec args);
enum CMM_SEMANTIC analyze_args(CMM_AST_NODE* node, struct AnalyCtxArgs args);
#pragma endregion

#pragma region Global States
/// 错误列表
CMM_SEMANTIC_ERROR semantic_errors[256];
size_t             semantic_errors_count = 0;
/// 一个哈希表，用来存放semantic的context
struct hashmap*    semantic_context      = NULL;
/// 一个链表，用来存放当前scope的名字。越是顶层越在后方
StringList*        semantic_scope        = NULL;
#pragma endregion


#pragma region Helper Functions
void free_semantic_ctx(void* data) {
    if (data == NULL) return;
    const SemanticContext* ctx = data;
    free(ctx->name);
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
    scope->data       = NULL;
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
    // 释放当前作用域的所有定义
    StringList* ptr = semantic_scope->data;
    while (ptr != NULL) {
        SemanticContext* deleted = (SemanticContext*)hashmap_delete(
            semantic_context, &(SemanticContext){.name = ptr->name});

        if (deleted != NULL) { free_semantic_ctx(deleted); }

        ptr = free_string_list_node(ptr);
    }
    semantic_scope = free_string_list_node(semantic_scope);
}

/// 在当前作用域下产生一个新的定义
/// @returns 1 成功
/// @returns 0 失败
int push_defination(const char* name, CMM_SEM_TYPE ty) {
    char* varname = cmm_concat_string(3, semantic_scope->name, ":def:", name);

    // 检查是否存在这个def
    if (hashmap_get(semantic_context, &(SemanticContext){.name = varname}) != NULL) {
        free(varname);
        return 0;
    }

    SemanticContext* ctx = (SemanticContext*)malloc(sizeof(SemanticContext));
    *ctx                 = (SemanticContext){.name = varname, .ty = ty};
    hashmap_set(semantic_context, ctx);

    StringList* def      = (StringList*)malloc(sizeof(StringList));
    def->name            = varname;
    def->back            = semantic_scope->data;
    def->ctx             = ctx;
    semantic_scope->data = def;

    return 1;
}

/// 在当前作用域下产生一个新的声明
void push_declaration(const char* name, CMM_SEM_TYPE ty) {
    StringList* def      = (StringList*)malloc(sizeof(StringList));
    def->name            = cmm_concat_string(3, semantic_scope->name, ":dec:", name);
    def->back            = semantic_scope->data;
    def->data            = NULL;
    semantic_scope->data = def;
}

void record_error(int lineno, enum CMM_SEMANTIC error) {
    // too many errors
    if (semantic_errors_count >= 256) { return; }
    semantic_errors[semantic_errors_count].line = lineno;
    semantic_errors[semantic_errors_count].type = error;
    semantic_errors_count++;
}

#pragma endregion

#pragma region Functions
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
    enum CMM_SEMANTIC x = analyze_program(node, (struct AnalyCtxProgram){});

    // clean
    hashmap_free(semantic_context);
    exit_semantic_scope();

    if (x != CMM_SE_OK) { return 1; }
    return 0;
}

/// Program: ExtDefList;
enum CMM_SEMANTIC analyze_program(CMM_AST_NODE* node, struct AnalyCtxProgram _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Program) { return CMM_SE_BAD_AST_TREE; }

    if (node->len != 1) { return CMM_SE_BAD_AST_TREE; }
    return analyze_ext_def_list(node->nodes, (struct AnalyCtxExtDefList){});
}

/// ExtDefList: /* empty */ | ExtDef ExtDefList
enum CMM_SEMANTIC analyze_ext_def_list(CMM_AST_NODE*             root,
                                       struct AnalyCtxExtDefList args) {
    CMM_AST_NODE* node = root;
    // 尾递归展开
    while (true) {
        // 结构性错误直接爆
        if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
        if (node->kind != CMM_TK_ExtDefList) { return CMM_SE_BAD_AST_TREE; }

        if (node->len == 0) { return CMM_SE_OK; }
        if (node->len == 2) {
            analyze_ext_def(node->nodes + 0, (struct AnalyCtxExtDef){});
            // TODO
            node = root->nodes + 1;
        }
        return CMM_SE_BAD_AST_TREE;
    }
}

/// ExtDef: Specifier ExtDecList SEMI
/// | Specifier SEMI
/// | Specifier FunDec CompSt
/// ;
/// 解析扩展的defination
enum CMM_SEMANTIC analyze_ext_def(CMM_AST_NODE* node, struct AnalyCtxExtDef _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_ExtDef) { return CMM_SE_BAD_AST_TREE; }

    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* decl      = node->nodes + 1;

    /// 无论这里有没有出错，我们让它继续往下走
    /// 错误的类型会被置为 error 这个特殊的原型类
    enum CMM_SEMANTIC spec_ok =
        analyze_specifier(specifier, (struct AnalyCtxSpecifier){});

    if (spec_ok != CMM_SE_OK) {
        specifier->context.kind           = CMM_AST_KIND_TYPE;
        specifier->context.data.type.kind = CMM_ERROR_TYPE;
    }

    if (decl->kind == CMM_TK_ExtDecList) {
        /// 变量定义
        // TODO
        analyze_ext_dec_list(
            decl, (struct AnalyCtxExtDecList){.ty = specifier->context.data.type});

    } else if (decl->kind == CMM_TK_FunDec) {
        // TODO
        analyze_fun_dec(decl, (struct AnalyCtxFunDec){});
    } else if (decl->kind == CMM_TK_SEMI) {
        /// 在这里声明了一个 Type
        /// TODO
        return CMM_SE_OK;
    }

    return CMM_SE_BAD_AST_TREE;
}

/// ExtDecList: VarDec | VarDec COMMA ExtDecList
enum CMM_SEMANTIC analyze_ext_dec_list(CMM_AST_NODE*             root,
                                       struct AnalyCtxExtDecList args) {
    CMM_AST_NODE* node = root;
    while (true) {
        // 结构性错误
        if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
        if (node->kind != CMM_TK_ExtDecList) { return CMM_SE_BAD_AST_TREE; }

        enum CMM_SEMANTIC vardec = analyze_var_dec(node->nodes + 0,
                                                   (struct AnalyCtxVarDec){
                                                       .ty    = args.ty,
                                                       .where = INSIDE_A_BLOCK,
                                                   });
        if (node->len == 1) {
            return CMM_SE_OK;
        } else if (node->len == 3) {
            node = node->nodes + 2;
        }
        return CMM_SE_BAD_AST_TREE;
    }
}


/// Specifier: TYPE | StructSpecifier
enum CMM_SEMANTIC analyze_specifier(CMM_AST_NODE* node, struct AnalyCtxSpecifier _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Specifier) { return CMM_SE_BAD_AST_TREE; }

    if (node->len != 1) { return CMM_SE_BAD_AST_TREE; }

    CMM_AST_NODE* inner = node->nodes;
    node->context.kind  = CMM_AST_KIND_TYPE;

    if (inner->kind == CMM_TK_TYPE) {
        node->context.data.type = cmm_ty_make_primitive(node->data.val_type);
        return CMM_SE_OK;
    } else if (inner->kind == CMM_TK_StructSpecifier) {
        enum CMM_SEMANTIC res =
            analyze_struct_specifier(node->nodes + 0, (struct AnalyCtxStructSpecifier){});
        if (res == CMM_SE_OK) {
            node->context.data.type = inner->context.data.type;
        } else {
            node->context.data.type = cmm_ty_make_error();
        }
    }

    return CMM_SE_BAD_AST_TREE;
}

/// StructSpecifier: STRUCT OptTag LC DefList RC | STRUCT Tag
enum CMM_SEMANTIC analyze_struct_specifier(CMM_AST_NODE*                  node,
                                           struct AnalyCtxStructSpecifier _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_StructSpecifier) { return CMM_SE_BAD_AST_TREE; }

    CMM_AST_NODE* tag = node->nodes + 1;

    if (node->len == 2) {
        ANALYZE_EXPECT_OK(analyze_tag(tag, (struct AnalyCtxTag){}));
        node->context.kind = CMM_AST_KIND_TYPE;
        /// TODO

    } else if (node->len == 4) {
        // OptTag LC DefList RC
        CMM_AST_NODE* opttag  = node->nodes + 1;
        CMM_AST_NODE* deflist = node->nodes + 3;

        ANALYZE_EXPECT_OK(analyze_opt_tag(opttag, (struct AnalyCtxOptTag){}));

        // 如果 struct 有名字
        if (opttag->context.kind == CMM_AST_KIND_IDENT) {
            const char* name = opttag->context.data.ident;
        }

        // TODO
        // 我们在 deflist 中 push 这个 defination
        analyze_def_list(deflist, (struct AnalyCtxDefList){});
    }

    return CMM_SE_BAD_AST_TREE;
}

/// Tag: empty | ID
enum CMM_SEMANTIC analyze_opt_tag(CMM_AST_NODE* node, struct AnalyCtxOptTag _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_OptTag) { return CMM_SE_BAD_AST_TREE; }
    if (node->len == 0) { return CMM_SE_OK; }

    if (node->len != 1) { return CMM_SE_BAD_AST_TREE; }
    CMM_AST_NODE* tag = node->nodes + 0;
    if (tag->kind != CMM_TK_Tag) { return CMM_SE_BAD_AST_TREE; }

    node->context.kind       = CMM_AST_KIND_IDENT;
    node->context.data.ident = tag->data.val_ident;
    return CMM_SE_OK;
};


/// Tag: ID
enum CMM_SEMANTIC analyze_tag(CMM_AST_NODE* node, struct AnalyCtxTag _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Tag) { return CMM_SE_BAD_AST_TREE; }
    if (node->len != 1) { return CMM_SE_BAD_AST_TREE; }

    CMM_AST_NODE* tag = node->nodes + 0;
    if (tag->kind != CMM_TK_ID) { return CMM_SE_BAD_AST_TREE; }

    node->context.kind       = CMM_AST_KIND_IDENT;
    node->context.data.ident = tag->data.val_ident;
    return CMM_SE_OK;
}

/// VarDec: ID | VarDec LB INT RB
/// TODO
enum CMM_SEMANTIC analyze_var_dec(CMM_AST_NODE* node, struct AnalyCtxVarDec args) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_VarDec) { return CMM_SE_BAD_AST_TREE; }

    if (node->len == 1) {
        // ID
        node->context.kind       = CMM_AST_KIND_IDENT;
        node->context.data.ident = node->nodes->data.val_ident;
        /// 注册这个变量

        int res = push_defination(node->nodes->data.val_ident, args.ty);
        if (res == 0) {
            node->context.data.type = args.ty;
        } else {
            switch (args.where) {
                case INSIDE_A_BLOCK:
                    record_error(node->location.line,
                                 CMM_SE_DUPLICATE_VARIABLE_DEFINATION);
                    break;
                case INSIDE_A_STRUCT:
                    record_error(node->location.line, CMM_SE_BAD_STRUCT_DOMAIN);
                    break;
            }
            node->context.data.type = cmm_ty_make_error();
        }

    } else if (node->len == 4) {
        // VarDec LB INT RB
        int           array_size = node->nodes[2].data.val_int;
        CMM_SEM_TYPE* ty         = malloc(sizeof(CMM_SEM_TYPE));
        *ty                      = args.ty;
        ANALYZE_EXPECT_OK(analyze_var_dec(
            node->nodes + 0,
            (struct AnalyCtxVarDec){.ty = cmm_ty_make_array(ty, array_size)}));
        // TODO
    }

    return CMM_SE_BAD_AST_TREE;
}

/// FunDec: ID LP VarList RP | ID LP RP
/// TODO
enum CMM_SEMANTIC analyze_fun_dec(CMM_AST_NODE* node, struct AnalyCtxFunDec _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_FunDec) { return CMM_SE_BAD_AST_TREE; }

    CMM_AST_NODE* id = node->nodes + 0;
    if (node->len == 3) {
        // TODO
    } else if (node->len == 4) {
        ANALYZE_EXPECT_OK(analyze_var_list(node->nodes + 2, (struct AnalyCtxVarList){}));
        // TODO
    } else {
        return CMM_SE_BAD_AST_TREE;
    }

    return CMM_SE_OK;
}

/// VarList: ParamDec COMMA VarList | ParamDec
/// TODO
enum CMM_SEMANTIC analyze_var_list(CMM_AST_NODE* root, struct AnalyCtxVarList _) {
    CMM_AST_NODE* node = root;
    while (true) {
        if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
        if (node->kind != CMM_TK_VarList) { return CMM_SE_BAD_AST_TREE; }

        // TODO
        if (node->len == 1) {
            // ParamDec
            return analyze_param_dec(node->nodes + 0, (struct AnalyCtxParamDec){});
        } else if (node->len == 3) {
            // ParamDec COMMA VarList
            ANALYZE_EXPECT_OK(
                analyze_param_dec(node->nodes + 0, (struct AnalyCtxParamDec){}));

            node = node->nodes + 2;
        }

        return CMM_SE_BAD_AST_TREE;
    }
}

/// ParamDec: Specifier VarDec
/// TODO
enum CMM_SEMANTIC analyze_param_dec(CMM_AST_NODE* node, struct AnalyCtxParamDec _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_ParamDec) { return CMM_SE_BAD_AST_TREE; }
    if (node->len != 2) return CMM_SE_BAD_AST_TREE;

    // TODO
    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* vardec    = node->nodes + 1;

    analyze_specifier(specifier, (struct AnalyCtxSpecifier){});
    analyze_var_dec(vardec, (struct AnalyCtxVarDec){});

    return CMM_SE_OK;
}

/// CompSt: LC DefList StmtList RC
/// TODO
enum CMM_SEMANTIC analyze_comp_st(CMM_AST_NODE* node, struct AnalyCtxCompSt _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_CompSt) { return CMM_SE_BAD_AST_TREE; }

    if (node->len != 4) { return CMM_SE_BAD_AST_TREE; }

    // LC DefList StmtList RC
    CMM_AST_NODE* deflist  = node->nodes + 1;
    CMM_AST_NODE* stmtlist = node->nodes + 2;

    // TODO
    analyze_def_list(deflist, (struct AnalyCtxDefList){});
    analyze_stmt_list(stmtlist, (struct AnalyCtxStmtList){});

    return CMM_SE_OK;
}

/// StmtList: /* empty */ | Stmt StmtList
/// TODO
enum CMM_SEMANTIC analyze_stmt_list(CMM_AST_NODE* root, struct AnalyCtxStmtList args) {
    CMM_AST_NODE* node = root;
    while (true) {
        if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
        if (node->kind != CMM_TK_StmtList) { return CMM_SE_BAD_AST_TREE; }

        if (node->len == 0) { return CMM_SE_OK; }

        if (node->len == 2) {
            // TODO
            // Stmt StmtList
            analyze_stmt(node->nodes + 0, (struct AnalyCtxStmt){});

            node = node->nodes + 1;
        }
        return CMM_SE_BAD_AST_TREE;
    }
}

// Stmt: Exp SEMI
//     | CompSt
//     | RETURN Exp SEMI
//     | IF LP Exp RP Stmt
//     | IF LP Exp RP Stmt ELSE Stmt
//     | WHILE LP Exp RP Stmt
/// TODO
enum CMM_SEMANTIC analyze_stmt(CMM_AST_NODE* node, struct AnalyCtxStmt _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Stmt) { return CMM_SE_BAD_AST_TREE; }

    // TODO
    return CMM_SE_BAD_AST_TREE;
}

/// DefList: /* empty */ | Def DefList
/// TODO
enum CMM_SEMANTIC analyze_def_list(CMM_AST_NODE* root, struct AnalyCtxDefList _) {
    CMM_AST_NODE* node = root;
    while (true) {
        if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
        if (node->kind != CMM_TK_DefList) { return CMM_SE_BAD_AST_TREE; }

        if (node->len == 0) { return CMM_SE_OK; }

        if (node->len == 2) {
            // TODO
            // Def DefList
            analyze_def(node->nodes + 0, (struct AnalyCtxDef){});

            node = node->nodes + 1;
        }

        return CMM_SE_BAD_AST_TREE;
    }
}

/// Def: Specifier DecList SEMI
/// TODO
enum CMM_SEMANTIC analyze_def(CMM_AST_NODE* node, struct AnalyCtxDef _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Def) { return CMM_SE_BAD_AST_TREE; }

    if (node->len != 3) { return CMM_SE_BAD_AST_TREE; }

    // Specifier DecList SEMI
    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* declist   = node->nodes + 1;

    // TODO
    analyze_specifier(specifier, (struct AnalyCtxSpecifier){});
    analyze_dec_list(declist, (struct AnalyCtxDecList){});

    return CMM_SE_OK;
}

/// DecList: Dec | Dec COMMA DecList
/// TODO
enum CMM_SEMANTIC analyze_dec_list(CMM_AST_NODE* root, struct AnalyCtxDecList _) {
    CMM_AST_NODE* node = root;
    while (true) {
        if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
        if (node->kind != CMM_TK_DecList) { return CMM_SE_BAD_AST_TREE; }

        if (node->len == 1) {
            // Dec
            return analyze_dec(node->nodes + 0, (struct AnalyCtxDec){});
        } else if (node->len == 3) {
            // Dec COMMA DecList
            analyze_dec(node->nodes + 0, (struct AnalyCtxDec){});

            node = node->nodes + 2;
        }
        return CMM_SE_BAD_AST_TREE;
    }
}

/// Dec: VarDec | VarDec ASSIGNOP Exp
/// TODO
enum CMM_SEMANTIC analyze_dec(CMM_AST_NODE* node, struct AnalyCtxDec _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Dec) { return CMM_SE_BAD_AST_TREE; }

    if (node->len == 1) {
        // VarDec
        return analyze_var_dec(node->nodes + 0, (struct AnalyCtxVarDec){});
    } else if (node->len == 3) {
        // VarDec ASSIGNOP Exp
        analyze_var_dec(node->nodes + 0, (struct AnalyCtxVarDec){});
        return analyze_exp(node->nodes + 2, (struct AnalyCtxExp){});
    }

    return CMM_SE_BAD_AST_TREE;
}


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
enum CMM_SEMANTIC analyze_exp(CMM_AST_NODE* node, struct AnalyCtxExp _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Exp) { return CMM_SE_BAD_AST_TREE; }

    // TODO

    return CMM_SE_BAD_AST_TREE;
}


// Args: Exp COMMA Args
//     | Exp
/// TODO
enum CMM_SEMANTIC analyze_args(CMM_AST_NODE* node, struct AnalyCtxArgs _) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Args) { return CMM_SE_BAD_AST_TREE; }

    if (node->len == 1) {
        // Exp
        return analyze_exp(node->nodes + 0, (struct AnalyCtxExp){});
    } else if (node->len == 3) {
        // Exp COMMA Args
        analyze_exp(node->nodes + 0, (struct AnalyCtxExp){});
        return analyze_args(node->nodes + 2, (struct AnalyCtxArgs){});
    }

    return CMM_SE_BAD_AST_TREE;
}
#pragma endregion