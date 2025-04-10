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

#define REPORT_AND_RETURN(e)                  \
    {                                         \
        record_error(node->location.line, e); \
        return e;                             \
    }

typedef struct SemanticContext {
    char*        name;
    CMM_SEM_TYPE ty;
    int          is_dec;
} SemanticContext;

typedef struct StringList {
    char*              name;
    struct StringList* back;
    union {
        struct StringList* data;
        SemanticContext*   ctx;
    };
} StringList;

enum VAR_WHERE {
    INSIDE_A_BLOCK,
    INSIDE_A_STRUCT,
} where;

struct AnalyCtxProgram {};
struct AnalyCtxExtDefList {};
struct AnalyCtxExtDef {};
struct AnalyCtxSpecifier {};
struct AnalyCtxExtDecList {
    CMM_SEM_TYPE ty;
};
struct AnalyCtxFunDec {
    CMM_SEM_TYPE return_ty;
};
struct AnalyCtxCompSt {};
struct AnalyCtxVarDec {
    enum VAR_WHERE where;
    CMM_SEM_TYPE   ty;
};
struct AnalyCtxStructSpecifier {};
struct AnalyCtxOptTag {};
struct AnalyCtxDefList {
    enum VAR_WHERE where;
};
struct AnalyCtxTag {};
struct AnalyCtxVarList {
    int*           list_len;
    CMM_SEM_TYPE** arg_types;
};
struct AnalyCtxParamDec {
    CMM_SEM_TYPE* ty;
};
struct AnalyCtxStmtList {};
struct AnalyCtxStmt {};
struct AnalyCtxExp {};
struct AnalyCtxDef {
    enum VAR_WHERE where;
};
struct AnalyCtxDecList {
    CMM_SEM_TYPE   ty;
    enum VAR_WHERE where;
};
struct AnalyCtxDec {
    CMM_SEM_TYPE   ty;
    enum VAR_WHERE where;
};
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

char* _ctx_finder(const char* name, StringList* scope) {
    return cmm_concat_string(3, semantic_scope->name, "::", name);
}

/// 在当前作用域下产生一个新的定义或声明
/// @returns 1 成功
/// @returns 0 失败
int _push_def_or_dec(const char* name, CMM_SEM_TYPE ty, int is_dec) {
    char* varname = _ctx_finder(name, semantic_scope);

    // 检查是否存在这个def
    const SemanticContext* existed =
        hashmap_get(semantic_context, &(SemanticContext){.name = varname});

    if (existed != NULL) {
        free(varname);
        if (is_dec) { return cmm_ty_eq(existed->ty, ty); }
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

/// 在当前作用域下产生一个新的定义
/// @returns 1 成功
/// @returns 0 失败
int push_defination(const char* name, CMM_SEM_TYPE ty) {
    return _push_def_or_dec(name, ty, 0);
}

/// 在当前作用域下产生一个新的声明
/// @returns 1 成功
/// @returns 0 失败
int push_declaration(const char* name, CMM_SEM_TYPE ty) {
    return _push_def_or_dec(name, ty, 1);
}

/// 在当前作用域，和它的上n级，获取一个定义
const SemanticContext* find_defination(const char* name) {
    const SemanticContext* ret   = NULL;
    StringList*            scope = semantic_scope;

    while (ret == NULL && scope != NULL) {
        char* varname = _ctx_finder(name, scope);
        ret = hashmap_get(semantic_context, &(SemanticContext){.name = varname});
        free(varname);
        scope = scope->back;
    }

    return ret;
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

        if (node->len == 0) {
            return CMM_SE_OK;
        } else if (node->len == 2) {
            analyze_ext_def(node->nodes + 0, (struct AnalyCtxExtDef){});
            // TODO
            node = root->nodes + 1;
        } else {
            return CMM_SE_BAD_AST_TREE;
        }
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
    enum CMM_SEMANTIC spec_ok = analyze_specifier(specifier,
                                                  (struct AnalyCtxSpecifier){

                                                  });

    if (spec_ok != CMM_SE_OK) {
        specifier->context.kind           = CMM_AST_KIND_TYPE;
        specifier->context.data.type.kind = CMM_ERROR_TYPE;
    }

    CMM_SEM_TYPE spec_ty = specifier->context.data.type;

    if (decl->kind == CMM_TK_ExtDecList) {
        /// 变量定义
        return analyze_ext_dec_list(decl, (struct AnalyCtxExtDecList){.ty = spec_ty});
    } else if (decl->kind == CMM_TK_FunDec) {
        /// 还需要分析函数体，所以不关心这里的错误
        enter_semantic_scope("fn");
        analyze_fun_dec(decl, (struct AnalyCtxFunDec){.return_ty = spec_ty});
        if (node->len == 3) {
            // FunDec CompSt
            analyze_comp_st(node->nodes + 2, (struct AnalyCtxCompSt){});
        } else if (node->len == 2) {
            // FunDec SEMI
            // just a declare
            // TODO
        }
        exit_semantic_scope();
        return CMM_SE_OK;
        // TODO
    } else if (decl->kind == CMM_TK_SEMI) {
        /// 在这里声明了一个 Type
        /// 期望在 Specifier 里已经注册了这个 Type
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
        } else {
            return CMM_SE_BAD_AST_TREE;
        }
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
        analyze_def_list(deflist, (struct AnalyCtxDefList){.where = INSIDE_A_STRUCT});
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
            node->context.data.type = cmm_ty_make_error();
            switch (args.where) {
                case INSIDE_A_BLOCK:
                    REPORT_AND_RETURN(CMM_SE_DUPLICATE_VARIABLE_DEFINATION);
                case INSIDE_A_STRUCT: REPORT_AND_RETURN(CMM_SE_BAD_STRUCT_DOMAIN);
            }
        }

    } else if (node->len == 4) {
        // VarDec LB INT RB
        int           array_size = node->nodes[2].data.val_int;
        CMM_SEM_TYPE* ty         = malloc(sizeof(CMM_SEM_TYPE));
        *ty                      = args.ty;
        ANALYZE_EXPECT_OK(analyze_var_dec(
            node->nodes + 0,
            (struct AnalyCtxVarDec){.ty = cmm_ty_make_array(ty, array_size)}));

        node->context.kind       = CMM_AST_KIND_IDENT;
        node->context.data.ident = node->nodes[2].context.data.ident;
        // TODO
    }

    return CMM_SE_BAD_AST_TREE;
}

/// FunDec: ID LP VarList RP | ID LP RP
/// TODO
enum CMM_SEMANTIC analyze_fun_dec(CMM_AST_NODE* node, struct AnalyCtxFunDec args) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_FunDec) { return CMM_SE_BAD_AST_TREE; }

    CMM_AST_NODE* id      = node->nodes + 0;
    char*         id_name = id->data.val_ident;

    CMM_SEM_TYPE fnty;
    if (node->len == 3) {
        CMM_SEM_TYPE* inner = malloc(sizeof(CMM_SEM_TYPE));
        // ID LP RP
        *inner              = args.return_ty;
        fnty                = cmm_ty_make_func(inner, 1);
    } else if (node->len == 4) {
        CMM_SEM_TYPE* inner    = NULL;
        int           list_len = 0;

        // 吞掉这里的错误
        analyze_var_list(node->nodes + 2,
                         (struct AnalyCtxVarList){
                             .list_len  = &list_len,
                             .arg_types = &inner,
                         });

        fnty = cmm_ty_make_func(inner, list_len);
    } else {
        return CMM_SE_BAD_AST_TREE;
    }

    // TODO 判断dec有误？
    push_declaration(id_name, fnty);

    node->context.kind      = CMM_AST_KIND_DECLARE;
    node->context.data.type = fnty;

    return CMM_SE_OK;
}

/// VarList: ParamDec COMMA VarList | ParamDec
/// TODO
enum CMM_SEMANTIC analyze_var_list(CMM_AST_NODE* root, struct AnalyCtxVarList args) {
    CMM_AST_NODE* node      = root;
    int           len       = 0;
    /// 都256个参数了不至于这都不够吧？？
    CMM_SEM_TYPE* arg_types = malloc(sizeof(CMM_SEM_TYPE) * 256);
    while (true) {
        if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
        if (node->kind != CMM_TK_VarList) { return CMM_SE_BAD_AST_TREE; }

        // TODO
        if (node->len != 1 && node->len != 3) { return CMM_SE_BAD_AST_TREE; }

        analyze_param_dec(node->nodes + 0,
                          (struct AnalyCtxParamDec){
                              .ty = arg_types + len,
                          });

        len++;

        if (len >= 255) {
            printf("panic: too many arguments");
            return CMM_SE_OK;
        }

        if (node->len == 1) {
            *args.list_len  = len;
            *args.arg_types = arg_types;
            return CMM_SE_OK;
        } else {
            node = node->nodes + 2;
        }
    }
}

/// ParamDec: Specifier VarDec
/// TODO
enum CMM_SEMANTIC analyze_param_dec(CMM_AST_NODE* node, struct AnalyCtxParamDec args) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_ParamDec) { return CMM_SE_BAD_AST_TREE; }
    if (node->len != 2) return CMM_SE_BAD_AST_TREE;

    // TODO
    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* vardec    = node->nodes + 1;

    enum CMM_SEMANTIC spec_ok =
        analyze_specifier(specifier, (struct AnalyCtxSpecifier){});

    if (spec_ok != CMM_SE_OK) {
        specifier->context.kind           = CMM_AST_KIND_TYPE;
        specifier->context.data.type.kind = CMM_ERROR_TYPE;
    }

    CMM_SEM_TYPE spec_ty = specifier->context.data.type;

    enum CMM_SEMANTIC var_ok = analyze_var_dec(vardec,
                                               (struct AnalyCtxVarDec){
                                                   .ty    = spec_ty,
                                                   .where = INSIDE_A_BLOCK,
                                               });

    if (var_ok != CMM_SE_OK) {
        *args.ty = cmm_ty_make_error();
        return var_ok;
    } else {
        const SemanticContext* ctx = find_defination(vardec->context.data.ident);
        *args.ty                   = ctx->ty;
        args.ty->bind              = vardec->context.data.ident;
    }

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

    enter_semantic_scope("&");

    // TODO
    analyze_def_list(deflist, (struct AnalyCtxDefList){});
    analyze_stmt_list(stmtlist, (struct AnalyCtxStmtList){});

    exit_semantic_scope();

    return CMM_SE_OK;
}

/// StmtList: /* empty */ | Stmt StmtList
/// TODO
enum CMM_SEMANTIC analyze_stmt_list(CMM_AST_NODE* root, struct AnalyCtxStmtList args) {
    CMM_AST_NODE* node = root;
    while (true) {
        if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
        if (node->kind != CMM_TK_StmtList) { return CMM_SE_BAD_AST_TREE; }

        if (node->len == 0) {
            return CMM_SE_OK;
        } else if (node->len == 2) {
            // TODO
            // Stmt StmtList
            analyze_stmt(node->nodes + 0, (struct AnalyCtxStmt){});

            node = node->nodes + 1;
        } else {
            return CMM_SE_BAD_AST_TREE;
        }
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
enum CMM_SEMANTIC analyze_def_list(CMM_AST_NODE* root, struct AnalyCtxDefList args) {
    CMM_AST_NODE* node = root;
    while (true) {
        if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
        if (node->kind != CMM_TK_DefList) { return CMM_SE_BAD_AST_TREE; }

        if (node->len == 0) {
            return CMM_SE_OK;
        } else if (node->len == 2) {
            // TODO
            // Def DefList
            analyze_def(node->nodes + 0, (struct AnalyCtxDef){.where = args.where});

            node = node->nodes + 1;
        } else {
            return CMM_SE_BAD_AST_TREE;
        }
    }
}

/// Def: Specifier DecList SEMI
/// TODO
enum CMM_SEMANTIC analyze_def(CMM_AST_NODE* node, struct AnalyCtxDef args) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Def) { return CMM_SE_BAD_AST_TREE; }

    if (node->len != 3) { return CMM_SE_BAD_AST_TREE; }

    // Specifier DecList SEMI
    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* declist   = node->nodes + 1;

    enum CMM_SEMANTIC spec_ok =
        analyze_specifier(specifier, (struct AnalyCtxSpecifier){});

    if (spec_ok != CMM_SE_OK) {
        specifier->context.kind           = CMM_AST_KIND_TYPE;
        specifier->context.data.type.kind = CMM_ERROR_TYPE;
    }

    CMM_SEM_TYPE spec_ty = specifier->context.data.type;

    analyze_dec_list(declist,
                     (struct AnalyCtxDecList){.ty = spec_ty, .where = args.where});

    return CMM_SE_OK;
}

/// DecList: Dec | Dec COMMA DecList
/// TODO
enum CMM_SEMANTIC analyze_dec_list(CMM_AST_NODE* root, struct AnalyCtxDecList args) {
    CMM_AST_NODE* node = root;
    while (true) {
        if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
        if (node->kind != CMM_TK_DecList) { return CMM_SE_BAD_AST_TREE; }

        if (node->len == 1) {
            // Dec
            analyze_dec(node->nodes + 0,
                        (struct AnalyCtxDec){.ty = args.ty, .where = args.where});
            return CMM_SE_OK;
        } else if (node->len == 3) {
            // Dec COMMA DecList
            analyze_dec(node->nodes + 0,
                        (struct AnalyCtxDec){.ty = args.ty, .where = args.where});

            node = node->nodes + 2;
        } else {
            return CMM_SE_BAD_AST_TREE;
        }
    }
}

/// Dec: VarDec | VarDec ASSIGNOP Exp
/// TODO
enum CMM_SEMANTIC analyze_dec(CMM_AST_NODE* node, struct AnalyCtxDec args) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Dec) { return CMM_SE_BAD_AST_TREE; }

    if (node->len != 1 && node->len != 3) { return CMM_SE_BAD_AST_TREE; }

    CMM_AST_NODE* vardec = node->nodes + 0;
    ANALYZE_EXPECT_OK(analyze_var_dec(
        vardec, (struct AnalyCtxVarDec){.where = args.where, .ty = args.ty}));

    if (node->len == 3) {
        if (args.where == INSIDE_A_STRUCT) {
            REPORT_AND_RETURN(CMM_SE_BAD_STRUCT_DOMAIN);
        }
        CMM_AST_NODE* exp = node->nodes + 2;
        analyze_exp(exp, (struct AnalyCtxExp){});
        // 赋值语句的类型检查
        if (!cmm_ty_fitable(exp->context.data.type, args.ty)) {
            REPORT_AND_RETURN(CMM_SE_ASSIGN_TYPE_ERROR);
        }
    }

    return CMM_SE_OK;
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
enum CMM_SEMANTIC analyze_exp(CMM_AST_NODE* node, struct AnalyCtxExp args) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_Exp) { return CMM_SE_BAD_AST_TREE; }

    node->context.kind = CMM_AST_KIND_TYPE;

    CMM_AST_NODE* a = node->nodes + 0;

    if (a->token == CMM_TK_Exp) {
        CMM_AST_NODE* op     = node->nodes + 1;
        CMM_AST_NODE* b      = node->nodes + 2;
        CMM_SEM_TYPE  type_a = a->context.data.type;

        analyze_exp(a, args);

        if (op->token == CMM_TK_DOT) {
            if (type_a.kind != CMM_PROD_TYPE) {
                REPORT_AND_RETURN(CMM_SE_BAD_STRUCT_ACCESS);
            }
            CMM_SEM_TYPE* ty = cmm_ty_field_of_struct(type_a, b->data.val_ident);
            if (ty == NULL) { REPORT_AND_RETURN(CMM_SE_UNDEFINED_STRUCT_DOMAIN); }
            node->context.data.type = *ty;
            return CMM_SE_OK;
        }

        analyze_exp(b, args);
        CMM_SEM_TYPE type_b = b->context.data.type;

        switch (op->token) {
            case CMM_TK_AND:
            case CMM_TK_OR:
            case CMM_TK_RELOP:
            case CMM_TK_PLUS:
            case CMM_TK_MINUS:
            case CMM_TK_STAR:
            case CMM_TK_DIV:
                if (!cmm_ty_fitable(type_a, type_b)) {
                    REPORT_AND_RETURN(CMM_SE_OPERAND_TYPE_ERROR);
                }
                node->context.data.type = type_b;
                break;
            case CMM_TK_LB:
                if (type_a.kind != CMM_ARRAY_TYPE) {
                    REPORT_AND_RETURN(CMM_SE_BAD_ARRAY_ACCESS);
                }
                if (type_b.kind != CMM_PRIMITIVE_TYPE ||
                    strcmp(type_b.name, "int") != 0) {
                    REPORT_AND_RETURN(CMM_SE_BAD_ARRAY_INDEX);
                }
                node->context.data.type = *type_a.inner;
                break;
            default: REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        }
    } else if (a->token == CMM_TK_ID) {
        CMM_AST_NODE* b = node->nodes + 2;
        // TODO function call
    } else if (a->token == CMM_TK_MINUS) {
        CMM_AST_NODE* b = node->nodes + 1;
        analyze_exp(b, args);
        CMM_SEM_TYPE type_b     = b->context.data.type;
        node->context.data.type = type_b;
    } else if (a->token == CMM_TK_NOT) {
        CMM_AST_NODE* b = node->nodes + 1;
        analyze_exp(b, args);
        CMM_SEM_TYPE type_b     = b->context.data.type;
        node->context.data.type = type_b;
    } else if (a->token == CMM_TK_LP) {
        CMM_AST_NODE* b = node->nodes + 1;
        analyze_exp(b, args);
        CMM_SEM_TYPE type_b     = b->context.data.type;
        node->context.data.type = type_b;
    } else if (a->token == CMM_TK_ID) {
        const SemanticContext* a_def = find_defination(a->data.val_ident);
        if (a_def == NULL) {
            node->context.data.type = cmm_ty_make_error();
        } else {
            node->context.data.type = a_def->ty;
        }
    } else if (a->token == CMM_TK_INT) {
        node->context.data.type = cmm_ty_make_primitive("int");
    } else if (a->token == CMM_TK_FLOAT) {
        node->context.data.type = cmm_ty_make_primitive("float");
    } else {
        return CMM_SE_BAD_AST_TREE;
    }

    return CMM_SE_OK;
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