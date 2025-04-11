#include "semantic.h"
#include "hashmap.h"
#include "predefines.h"
#include "syndef.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#pragma region Definations
#define ANALYZE_EXPECT_OK(x)                          \
    {                                                 \
        enum CMM_SEMANTIC ret = x;                    \
        if (ret != CMM_SE_OK) RETURN_WITH_TRACE(ret); \
    }

#define REPORT_AND_RETURN(e)                                                           \
    {                                                                                  \
        if (e == CMM_SE_BAD_AST_TREE) {                                                \
            printf(                                                                    \
                "bad ast tree occurred in %s: %s:%d\n", __func__, __FILE__, __LINE__); \
        }                                                                              \
        record_error(node->location.line, e);                                          \
        return e;                                                                      \
    }

#ifdef CMM_DEBUG_FLAGTRACE
int __sem_trace_spaces = 0;
#    define FUNCTION_TRACE                                                               \
        {                                                                                \
            __sem_trace_spaces++;                                                        \
            for (int i = 0; i < __sem_trace_spaces; i++) printf(" ");                    \
            printf("\033[1;34m  >== %s : %s:%d\n\033[0m", __func__, __FILE__, __LINE__); \
        }
#    define RETURN_WITH_TRACE(x)  \
        {                         \
            __sem_trace_spaces--; \
            return x;             \
        }
#else
#    define FUNCTION_TRACE \
        {}
#    define RETURN_WITH_TRACE(x) \
        { return x; }
#endif


typedef struct SemanticContext {
    char*        name;
    CMM_SEM_TYPE ty;
    enum {
        SEM_CTX_DEFINATION,
        SEM_CTX_DECLARE,
    } def;
    enum SemContextKind {
        SEM_CTX_TYPE,
        SEM_CTX_VAR,
    } kind;
} SemanticContext;

typedef struct StringList {
    char*              name;
    struct StringList* back;
    struct StringList* data;
    SemanticContext*   ctx;
} StringList;

enum VAR_WHERE {
    INSIDE_A_BLOCK,
    INSIDE_A_STRUCT,
} where;

struct AnalyCtxProgram {
    int _void;
};
struct AnalyCtxExtDefList {
    int _void;
};
struct AnalyCtxExtDef {
    int _void;
};
struct AnalyCtxSpecifier {
    int _void;
};
struct AnalyCtxExtDecList {
    CMM_SEM_TYPE ty;
};
struct AnalyCtxFunDec {
    CMM_SEM_TYPE return_ty;
    int          is_def;
};
struct AnalyCtxCompSt {
    CMM_SEM_TYPE current_fn_ty;
};
struct AnalyCtxVarDec {
    enum VAR_WHERE where;
    CMM_SEM_TYPE   ty;
};
struct AnalyCtxStructSpecifier {
    int _void;
};
struct AnalyCtxOptTag {
    int _void;
};
struct AnalyCtxDefList {
    enum VAR_WHERE where;
    int*           struct_fields_len;
    CMM_SEM_TYPE** struct_fields_types;
};
struct AnalyCtxTag {
    int _void;
};
struct AnalyCtxVarList {
    int*           list_len;
    CMM_SEM_TYPE** arg_types;
};
struct AnalyCtxParamDec {
    CMM_SEM_TYPE* ty;
};
struct AnalyCtxStmtList {
    CMM_SEM_TYPE current_fn_ty;
};
struct AnalyCtxStmt {
    CMM_SEM_TYPE current_fn_ty;
};
struct AnalyCtxExp {
    int _void;
};
struct AnalyCtxDef {
    enum VAR_WHERE where;
    CMM_SEM_TYPE*  fill_into;
    int*           offset;
};
struct AnalyCtxDecList {
    CMM_SEM_TYPE   ty;
    enum VAR_WHERE where;
    CMM_SEM_TYPE*  fill_into;
    int*           offset;
};
struct AnalyCtxDec {
    CMM_SEM_TYPE   ty;
    enum VAR_WHERE where;
    CMM_SEM_TYPE*  fill_into;
    int*           offset;
};
struct AnalyCtxArgs {
    CMM_SEM_TYPE calling;
};

const SemanticContext* find_defination(const char* name);

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
CMM_SEMANTIC_ERROR semantic_errors[65535];
size_t             semantic_errors_count = 0;
/// 一个哈希表，用来存放semantic的context
struct hashmap*    semantic_context      = NULL;
/// 一个链表，用来存放当前scope的名字。越是顶层越在后方
StringList*        semantic_scope        = NULL;
StringList*        root_semantic_scope   = NULL;
#pragma endregion


#pragma region Helper Functions

const char* cmm_semantic_error_to_string(enum CMM_SEMANTIC type) {
    switch (type) {
        case CMM_SE_BAD_AST_TREE: return "unexpected bad ast tree";
        case CMM_SE_OK: return "ok";
        case CMM_SE_UNDEFINED_VARIABLE: return "Undefined variable";
        case CMM_SE_UNDEFINED_FUNCTION: return "Undefined function";
        case CMM_SE_DUPLICATE_VARIABLE_DEFINATION: return "Duplicate variable definition";
        case CMM_SE_DUPLICATE_FUNCTION_DEFINATION: return "Duplicate function definition";
        case CMM_SE_ASSIGN_TYPE_ERROR: return "Assignment type error";
        case CMM_SE_ASSIGN_TO_RVALUE: return "Assignment to rvalue";
        case CMM_SE_OPERAND_TYPE_ERROR: return "Operand type error";
        case CMM_SE_RETURN_TYPE_ERROR: return "Return type error";
        case CMM_SE_ARGS_NOT_MATCH: return "Arguments not match";
        case CMM_SE_BAD_ARRAY_ACCESS: return "Bad array access";
        case CMM_SE_BAD_FUNCTION_CALL: return "Bad function call";
        case CMM_SE_BAD_ARRAY_INDEX: return "Bad array index";
        case CMM_SE_BAD_STRUCT_ACCESS: return "Bad struct access";
        case CMM_SE_UNDEFINED_STRUCT_DOMAIN: return "Undefined struct domain";
        case CMM_SE_BAD_STRUCT_DOMAIN: return "Bad struct domain";
        case CMM_SE_DUPLICATE_STRUCT: return "Duplicate struct definition";
        case CMM_SE_UNDEFINED_STRUCT: return "Undefined struct";
        case CMM_SE_FUNCTION_DECLARED_NOT_DEFINED:
            return "Function declared but not defined";
        case CMM_SE_CONFLICT_FUNCTION_DECLARATION: return "Function declaration conflict";
        default: return "Unknown semantic error";
    }
}

char* gen_unnamed_struct_name() {
    static int unnamed_struct_count = 0;
    char*      name                 = malloc(50);
    sprintf(name, "(unnamed)struct_%d", unnamed_struct_count++);
    return name;
}

void free_semantic_ctx(void* data) {
    // if (data == NULL) return;
    // const SemanticContext* ctx = data;
    // free(ctx->name);
    (void)data;
}

int semantic_ctx_compare(const void* a, const void* b, void* _udata) {
    const SemanticContext* ua = a;
    const SemanticContext* ub = b;
    (void)(_udata);
    return strcmp(ua->name, ub->name);
}

uint64_t senamtic_ctx_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const SemanticContext* ctx = item;
    return hashmap_sip(ctx->name, strlen(ctx->name), seed0, seed1);
}


/// 进入一个新的语义分析作用域
StringList* enter_semantic_scope(const char* name) {
    StringList* scope = (StringList*)malloc(sizeof(StringList));
    scope->name       = cmm_clone_string(name);
    scope->data       = NULL;
    scope->back       = semantic_scope;
    semantic_scope    = scope;

#ifdef CMM_DEBUG_FLAGTRACE
    printf("\033[1;32mentering scope: %s\033[0m\n", name);
#endif

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
#ifdef CMM_DEBUG_FLAGTRACE
    printf("\033[1;32mquiting scope: %s\033[0m\n", semantic_scope->name);
#endif
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
    if (scope == NULL) {
        return cmm_clone_string(name);
    } else {
        char* tmp = _ctx_finder(scope->name, scope->back);
        if (name == NULL) { return tmp; }
        char* ret = cmm_concat_string(3, tmp, "::", name);
        free(tmp);
        return ret;
    }
}

/// 在当前作用域下产生一个新的定义或声明
/// @returns 1 成功
/// @returns 0 失败
int push_context(SemanticContext arg) {
    char* varname = _ctx_finder(arg.name, semantic_scope);

    // 检查是否存在这个def
    const SemanticContext* existed =
        hashmap_get(semantic_context, &(SemanticContext){.name = varname});

    if (existed != NULL) {
#ifdef CMM_DEBUG_FLAGTRACE
        if (arg.def == SEM_CTX_DECLARE || existed->def == SEM_CTX_DECLARE)
            printf("\033[1;33m[register] %s %s : %s\033[0m\n",
                   arg.def == SEM_CTX_DECLARE ? "dec" : "def",
                   varname,
                   arg.ty.name);
        else
            printf("\033[1;43mERROR [register again] %s %s : %s\033[0m\n",
                   arg.def == SEM_CTX_DECLARE ? "dec" : "def",
                   varname,
                   arg.ty.name);
#endif
        free(varname);
        if (arg.def == SEM_CTX_DECLARE || existed->def == SEM_CTX_DECLARE) {
            return cmm_ty_eq(existed->ty, arg.ty);
        }
        return 0;
    }

    /// 变量的名字不能和类型一样
    const SemanticContext* existed_rec = find_defination(arg.name);
    if (existed_rec != NULL && existed_rec->kind == SEM_CTX_TYPE &&
        arg.kind == SEM_CTX_VAR) {
        return 0;
    }

#ifdef CMM_DEBUG_FLAGTRACE
    printf("\033[1;33m[register] %s %s : %s\033[0m\n",
           arg.def == SEM_CTX_DECLARE ? "dec" : "def",
           varname,
           arg.ty.name);
#endif

    SemanticContext* allo_ctx = (SemanticContext*)malloc(sizeof(SemanticContext));
    *allo_ctx                 = arg;
    allo_ctx->name            = varname;
    hashmap_set(semantic_context, allo_ctx);

    StringList* def      = (StringList*)malloc(sizeof(StringList));
    def->name            = varname;
    def->back            = semantic_scope->data;
    def->ctx             = allo_ctx;
    semantic_scope->data = def;

    return 1;
}

/// 在当前作用域，和它的上n级，获取一个定义
const SemanticContext* find_defination(const char* name) {
    const SemanticContext* ret   = NULL;
    StringList*            scope = semantic_scope;

    while (ret == NULL && scope != NULL) {
        char* varname = _ctx_finder(name, scope);
#ifdef CMM_DEBUG_FLAGTRACE
        printf("[search] finding %s\n", varname);
#endif
        ret = hashmap_get(semantic_context, &(SemanticContext){.name = varname});
        free(varname);
        scope = scope->back;
    }

#ifdef CMM_DEBUG_FLAGTRACE
    if (ret == NULL) {
        printf("[search] not found\n");
    } else {
        printf("[search] found: ty = %s\n", ret->ty.name);
    }
#endif

    return ret;
}

void record_error(int lineno, enum CMM_SEMANTIC error) {
#ifdef CMM_DEBUG_FLAGTRACE
    printf("\033[1;31mError type %d at Line %d: %s\033[0m\n",
           error,
           lineno,
           cmm_semantic_error_to_string(error));
#endif

    // too many errors
    if (semantic_errors_count >= 65535) { return; }
    semantic_errors[semantic_errors_count].line = lineno;
    semantic_errors[semantic_errors_count].type = error;
    semantic_errors_count++;
}

#pragma endregion

#pragma region Functions

CMM_SEMANTIC_ERROR* cmm_get_semantic_errors() { return semantic_errors; }

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
    root_semantic_scope = semantic_scope;

    /// analyze
    analyze_program(node, (struct AnalyCtxProgram){._void = 0});

    // clean
    // hashmap_free(semantic_context);
    exit_semantic_scope();

    return semantic_errors_count;
}

/// Program: ExtDefList;
enum CMM_SEMANTIC analyze_program(CMM_AST_NODE* node, struct AnalyCtxProgram _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_Program) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->len != 1) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    RETURN_WITH_TRACE(
        analyze_ext_def_list(node->nodes, (struct AnalyCtxExtDefList){._void = 0}));
}

/// ExtDefList: /* empty */ | ExtDef ExtDefList
enum CMM_SEMANTIC analyze_ext_def_list(CMM_AST_NODE* root, struct AnalyCtxExtDefList _) {
    (void)_;
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;
    // 尾递归展开
    while (true) {
        // 结构性错误直接爆
        if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        if (node->token != CMM_TK_ExtDefList) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

        if (node->len == 0) {
            RETURN_WITH_TRACE(CMM_SE_OK);
        } else if (node->len == 2) {
            analyze_ext_def(node->nodes + 0, (struct AnalyCtxExtDef){._void = 0});
            // TODO
            node = node->nodes + 1;
        } else {
            REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        }
    }
}

/// ExtDef: Specifier ExtDecList SEMI
/// | Specifier SEMI
/// | Specifier FunDec CompSt
/// ;
/// 解析扩展的defination
enum CMM_SEMANTIC analyze_ext_def(CMM_AST_NODE* node, struct AnalyCtxExtDef _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_ExtDef) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* decl      = node->nodes + 1;

    /// 无论这里有没有出错，我们让它继续往下走
    /// 错误的类型会被置为 error 这个特殊的原型类
    enum CMM_SEMANTIC spec_ok =
        analyze_specifier(specifier, (struct AnalyCtxSpecifier){._void = 0});

    if (spec_ok != CMM_SE_OK) {
        specifier->context.kind      = CMM_AST_KIND_TYPE;
        specifier->context.data.type = cmm_ty_make_error();
    }

    CMM_SEM_TYPE spec_ty = specifier->context.data.type;

#ifdef CMM_DEBUG_FLAGTRACE
    printf("type is %s\n", spec_ty.name);
#endif

    if (decl->token == CMM_TK_ExtDecList) {
        /// 变量定义
        analyze_ext_dec_list(decl, (struct AnalyCtxExtDecList){.ty = spec_ty});
        RETURN_WITH_TRACE(CMM_SE_OK);
    } else if (decl->token == CMM_TK_FunDec) {
        /// 还需要分析函数体，所以不关心这里的错误
        /// decl->nodes 是一个 &ID，否则AST错误
        char* fn_name = decl->nodes->data.val_ident;
        enter_semantic_scope(fn_name);
        analyze_fun_dec(decl,
                        (struct AnalyCtxFunDec){
                            .return_ty = spec_ty,
                            .is_def    = node->len == 3,
                        });
        if (node->len == 3) {
            // FunDec CompSt
            analyze_comp_st(
                node->nodes + 2,
                (struct AnalyCtxCompSt){.current_fn_ty = decl->context.data.type});
        } else if (node->len == 2) {
            // FunDec SEMI
            // just a declare
            // TODO
        }
        exit_semantic_scope();
        RETURN_WITH_TRACE(CMM_SE_OK);
        // TODO
    } else if (decl->token == CMM_TK_SEMI) {
        /// 在这里声明了一个 Type
        /// 期望在 Specifier 里已经注册了这个 Type
        RETURN_WITH_TRACE(CMM_SE_OK);
    }

    REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
}

/// ExtDecList: VarDec | VarDec COMMA ExtDecList
enum CMM_SEMANTIC analyze_ext_dec_list(CMM_AST_NODE*             root,
                                       struct AnalyCtxExtDecList args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;
    while (true) {
        // 结构性错误
        if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        if (node->token != CMM_TK_ExtDecList) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

        analyze_var_dec(node->nodes + 0,
                        (struct AnalyCtxVarDec){
                            .ty    = args.ty,
                            .where = INSIDE_A_BLOCK,
                        });
        if (node->len == 1) {
            RETURN_WITH_TRACE(CMM_SE_OK);
        } else if (node->len == 3) {
            node = node->nodes + 2;
        } else {
            REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        }
    }
}


/// Specifier: TYPE | StructSpecifier
enum CMM_SEMANTIC analyze_specifier(CMM_AST_NODE* node, struct AnalyCtxSpecifier _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_Specifier) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->len != 1) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    CMM_AST_NODE* inner = node->nodes;
    node->context.kind  = CMM_AST_KIND_TYPE;

    if (inner->token == CMM_TK_TYPE) {
        node->context.data.type = cmm_ty_make_primitive(inner->data.val_type);
        RETURN_WITH_TRACE(CMM_SE_OK);
    } else if (inner->token == CMM_TK_StructSpecifier) {
        enum CMM_SEMANTIC res =
            analyze_struct_specifier(inner, (struct AnalyCtxStructSpecifier){._void = 0});
        if (res == CMM_SE_OK) {
            node->context.data.type = inner->context.data.type;
        } else {
            node->context.data.type = cmm_ty_make_error();
        }
        RETURN_WITH_TRACE(CMM_SE_OK);
    }

    REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
}

/// StructSpecifier: STRUCT OptTag LC DefList RC | STRUCT Tag
enum CMM_SEMANTIC analyze_struct_specifier(CMM_AST_NODE*                  node,
                                           struct AnalyCtxStructSpecifier _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_StructSpecifier) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    CMM_AST_NODE* tag = node->nodes + 1;

    if (node->len == 2) {
        ANALYZE_EXPECT_OK(analyze_tag(tag, (struct AnalyCtxTag){._void = 0}));
        node->context.kind         = CMM_AST_KIND_TYPE;
        const SemanticContext* ctx = find_defination(tag->context.data.ident);
        if (ctx == NULL) {
            node->context.data.type = cmm_ty_make_error();
            REPORT_AND_RETURN(CMM_SE_UNDEFINED_STRUCT);
        } else {
            node->context.data.type = ctx->ty;
        }
        /// TODO
    } else if (node->len == 5) {
        //  STRUCT OptTag LC DefList RC
        CMM_AST_NODE* opttag  = node->nodes + 1;
        CMM_AST_NODE* deflist = node->nodes + 3;

        ANALYZE_EXPECT_OK(analyze_opt_tag(opttag, (struct AnalyCtxOptTag){._void = 0}));

        // 如果 struct 有名字
        char* name = opttag->context.kind == CMM_AST_KIND_IDENT
                         ? opttag->context.data.ident
                         : gen_unnamed_struct_name();
        enter_semantic_scope(name);

        CMM_SEM_TYPE* inner = NULL;
        int           size  = 0;

        analyze_def_list(deflist,
                         (struct AnalyCtxDefList){
                             .where               = INSIDE_A_STRUCT,
                             .struct_fields_len   = &size,
                             .struct_fields_types = &inner,
                         });

        exit_semantic_scope();
        CMM_SEM_TYPE ty = cmm_ty_make_struct(name, inner, size);

        /// struct 会被提升到顶层
        StringList* current_scope = semantic_scope;
        semantic_scope            = root_semantic_scope;
        int ok                    = push_context((SemanticContext){
                               .def  = SEM_CTX_DEFINATION,
                               .kind = SEM_CTX_TYPE,
                               .name = name,
                               .ty   = ty,
        });
        semantic_scope            = current_scope;
        node->context.data.type   = ty;

        if (!ok) { REPORT_AND_RETURN(CMM_SE_DUPLICATE_STRUCT); }
    } else {
        REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    }

    RETURN_WITH_TRACE(CMM_SE_OK);
}

/// Tag: empty | ID
enum CMM_SEMANTIC analyze_opt_tag(CMM_AST_NODE* node, struct AnalyCtxOptTag _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_OptTag) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->len == 0) { RETURN_WITH_TRACE(CMM_SE_OK); }

    if (node->len != 1) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    CMM_AST_NODE* tag = node->nodes + 0;
    if (tag->token != CMM_TK_ID) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    node->context.kind       = CMM_AST_KIND_IDENT;
    node->context.data.ident = tag->data.val_ident;
    RETURN_WITH_TRACE(CMM_SE_OK);
}


/// Tag: ID
enum CMM_SEMANTIC analyze_tag(CMM_AST_NODE* node, struct AnalyCtxTag _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_Tag) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->len != 1) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    CMM_AST_NODE* tag = node->nodes + 0;
    if (tag->token != CMM_TK_ID) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    node->context.kind       = CMM_AST_KIND_IDENT;
    node->context.data.ident = tag->data.val_ident;
    RETURN_WITH_TRACE(CMM_SE_OK);
}

/// VarDec: ID | VarDec LB INT RB
/// TODO
enum CMM_SEMANTIC analyze_var_dec(CMM_AST_NODE* node, struct AnalyCtxVarDec args) {
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_VarDec) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    if (node->len == 1) {
        // ID
        node->context.kind       = CMM_AST_KIND_IDENT;
        node->context.data.ident = node->nodes->data.val_ident;
        /// 注册这个变量
        args.ty.bind             = node->context.data.ident;
        int ok                   = push_context((SemanticContext){
                              .def  = SEM_CTX_DEFINATION,
                              .kind = SEM_CTX_VAR,
                              .name = node->nodes->data.val_ident,
                              .ty   = args.ty,
        });
        if (ok) {
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
        CMM_AST_NODE* vardec     = node->nodes + 0;
        ANALYZE_EXPECT_OK(analyze_var_dec(
            vardec,
            (struct AnalyCtxVarDec){.where = args.where,
                                    .ty    = cmm_ty_make_array(ty, array_size)}));

        node->context.kind       = CMM_AST_KIND_IDENT;
        node->context.data.ident = vardec->context.data.ident;
        node->context.data.type  = vardec->context.data.type;
        // TODO
    } else {
        REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    }

    RETURN_WITH_TRACE(CMM_SE_OK);
}

/// FunDec: ID LP VarList RP | ID LP RP
/// TODO
enum CMM_SEMANTIC analyze_fun_dec(CMM_AST_NODE* node, struct AnalyCtxFunDec args) {
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_FunDec) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

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

        inner[list_len++] = args.return_ty;

        fnty = cmm_ty_make_func(inner, list_len);
    } else {
        REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    }

    node->context.kind       = CMM_AST_KIND_DECLARE;
    node->context.data.type  = fnty;
    node->context.data.ident = id_name;

    // TODO 判断dec有误？
    {
        /// 函数定义在上层作用域
        /// 先修改当前作用域，然后再恢复
        StringList* current_scope = semantic_scope;
        semantic_scope            = semantic_scope->back;
        int ret                   = push_context((SemanticContext){
                              .def  = args.is_def ? SEM_CTX_DEFINATION : SEM_CTX_DECLARE,
                              .kind = SEM_CTX_VAR,
                              .name = id_name,
                              .ty   = fnty,
        });
        semantic_scope            = current_scope;

        if (ret == 0) {
            if (args.is_def) {
                /// 父作用域定义重复，为了继续分析 comst，我们再
                /// push一个defination到子作用域
                push_context((SemanticContext){
                    .def  = SEM_CTX_DEFINATION,
                    .kind = SEM_CTX_VAR,
                    .name = id_name,
                    .ty   = fnty,
                });
                REPORT_AND_RETURN(CMM_SE_DUPLICATE_FUNCTION_DEFINATION);
            } else {
                REPORT_AND_RETURN(CMM_SE_CONFLICT_FUNCTION_DECLARATION);
            }
        }
    }

    RETURN_WITH_TRACE(CMM_SE_OK);
}

/// VarList: ParamDec COMMA VarList | ParamDec
/// TODO
enum CMM_SEMANTIC analyze_var_list(CMM_AST_NODE* root, struct AnalyCtxVarList args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node      = root;
    int           len       = 0;
    /// 都256个参数了不至于这都不够吧？？
    CMM_SEM_TYPE* arg_types = malloc(sizeof(CMM_SEM_TYPE) * 256);
    while (true) {
        if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        if (node->token != CMM_TK_VarList) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

        // TODO
        if (node->len != 1 && node->len != 3) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

        analyze_param_dec(node->nodes + 0,
                          (struct AnalyCtxParamDec){
                              .ty = arg_types + len,
                          });

        len++;

        if (len >= 255) {
            printf("panic: too many arguments");
            RETURN_WITH_TRACE(CMM_SE_OK);
        }

        if (node->len == 1) {
            *args.list_len  = len;
            *args.arg_types = arg_types;
            RETURN_WITH_TRACE(CMM_SE_OK);
        } else {
            node = node->nodes + 2;
        }
    }
}

/// ParamDec: Specifier VarDec
/// TODO
enum CMM_SEMANTIC analyze_param_dec(CMM_AST_NODE* node, struct AnalyCtxParamDec args) {
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_ParamDec) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->len != 2) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    // TODO
    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* vardec    = node->nodes + 1;

    enum CMM_SEMANTIC spec_ok =
        analyze_specifier(specifier, (struct AnalyCtxSpecifier){._void = 0});

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
        RETURN_WITH_TRACE(var_ok);
    } else {
        const SemanticContext* ctx = find_defination(vardec->context.data.ident);
        *args.ty                   = ctx->ty;
        args.ty->bind              = vardec->context.data.ident;
    }

    RETURN_WITH_TRACE(CMM_SE_OK);
}

/// CompSt: LC DefList StmtList RC
/// TODO
enum CMM_SEMANTIC analyze_comp_st(CMM_AST_NODE* node, struct AnalyCtxCompSt args) {
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_CompSt) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    if (node->len != 4) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    // LC DefList StmtList RC
    CMM_AST_NODE* deflist  = node->nodes + 1;
    CMM_AST_NODE* stmtlist = node->nodes + 2;

    enter_semantic_scope("&");

    // TODO
    analyze_def_list(deflist, (struct AnalyCtxDefList){.where = INSIDE_A_BLOCK});
    analyze_stmt_list(stmtlist,
                      (struct AnalyCtxStmtList){.current_fn_ty = args.current_fn_ty});

    exit_semantic_scope();

    RETURN_WITH_TRACE(CMM_SE_OK);
}

/// StmtList: /* empty */ | Stmt StmtList
/// TODO
enum CMM_SEMANTIC analyze_stmt_list(CMM_AST_NODE* root, struct AnalyCtxStmtList args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;
    while (true) {
        if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        if (node->token != CMM_TK_StmtList) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

        if (node->len == 0) {
            RETURN_WITH_TRACE(CMM_SE_OK);
        } else if (node->len == 2) {
            // TODO
            // Stmt StmtList
            analyze_stmt(node->nodes + 0,
                         (struct AnalyCtxStmt){.current_fn_ty = args.current_fn_ty});

            node = node->nodes + 1;
        } else {
            REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
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
enum CMM_SEMANTIC analyze_stmt(CMM_AST_NODE* node, struct AnalyCtxStmt args) {
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_Stmt) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    CMM_AST_NODE* first = node->nodes + 0;
    switch (first->token) {
        case CMM_TK_Exp: analyze_exp(first, (struct AnalyCtxExp){._void = 0}); break;
        case CMM_TK_CompSt:
            analyze_comp_st(first,
                            (struct AnalyCtxCompSt){.current_fn_ty = args.current_fn_ty});
            break;
        case CMM_TK_RETURN: {
            CMM_AST_NODE* exp = node->nodes + 1;
            analyze_exp(exp, (struct AnalyCtxExp){._void = 0});
            CMM_SEM_TYPE need = args.current_fn_ty.inner[args.current_fn_ty.size - 1];
            CMM_SEM_TYPE got  = exp->context.data.type;
            if (!cmm_ty_fitable(need, got)) {
#ifdef CMM_DEBUG_FLAGTRACE
                printf("need %s, got %s\n", need.name, got.name);
#endif
                REPORT_AND_RETURN(CMM_SE_RETURN_TYPE_ERROR)
            }
            break;
        }
        case CMM_TK_IF: {
            CMM_AST_NODE* exp = node->nodes + 2;
            analyze_exp(exp, (struct AnalyCtxExp){._void = 0});
            analyze_stmt(node->nodes + 4,
                         (struct AnalyCtxStmt){.current_fn_ty = args.current_fn_ty});
            if (node->len == 7) {
                // IF LP Exp RP Stmt ELSE Stmt
                analyze_stmt(node->nodes + 6,
                             (struct AnalyCtxStmt){.current_fn_ty = args.current_fn_ty});
            }
            break;
        }
        case CMM_TK_WHILE: {
            CMM_AST_NODE* exp = node->nodes + 2;
            analyze_exp(exp, (struct AnalyCtxExp){._void = 0});
            analyze_stmt(node->nodes + 4,
                         (struct AnalyCtxStmt){.current_fn_ty = args.current_fn_ty});
            break;
        }
        default: {
            REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
            break;
        }
    }
    RETURN_WITH_TRACE(CMM_SE_OK);
}

/// DefList: /* empty */ | Def DefList
/// TODO
enum CMM_SEMANTIC analyze_def_list(CMM_AST_NODE* root, struct AnalyCtxDefList args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;

    CMM_SEM_TYPE* fields     = malloc(sizeof(CMM_SEM_TYPE) * 500);
    int           fileds_len = 0;

    while (true) {
        if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        if (node->token != CMM_TK_DefList) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

        if (node->len == 0) {
            if (args.where == INSIDE_A_STRUCT) {
                *args.struct_fields_len   = fileds_len;
                *args.struct_fields_types = fields;
            } else {
                free(fields);
            }
            RETURN_WITH_TRACE(CMM_SE_OK);
        } else if (node->len == 2) {
            // TODO
            // Def DefList
            analyze_def(
                node->nodes + 0,
                (struct AnalyCtxDef){
                    .where = args.where, .fill_into = fields, .offset = &fileds_len});

            node = node->nodes + 1;
        } else {
            REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        }
    }
}

/// Def: Specifier DecList SEMI
/// TODO
enum CMM_SEMANTIC analyze_def(CMM_AST_NODE* node, struct AnalyCtxDef args) {
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_Def) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    if (node->len != 3) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    // Specifier DecList SEMI
    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* declist   = node->nodes + 1;

    enum CMM_SEMANTIC spec_ok =
        analyze_specifier(specifier, (struct AnalyCtxSpecifier){._void = 0});

    if (spec_ok != CMM_SE_OK) {
        specifier->context.kind           = CMM_AST_KIND_TYPE;
        specifier->context.data.type.kind = CMM_ERROR_TYPE;
    }

    CMM_SEM_TYPE spec_ty = specifier->context.data.type;

    analyze_dec_list(declist,
                     (struct AnalyCtxDecList){.ty        = spec_ty,
                                              .where     = args.where,
                                              .fill_into = args.fill_into,
                                              .offset    = args.offset});

    RETURN_WITH_TRACE(CMM_SE_OK);
}

/// DecList: Dec | Dec COMMA DecList
/// TODO
enum CMM_SEMANTIC analyze_dec_list(CMM_AST_NODE* root, struct AnalyCtxDecList args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;
    while (true) {
        if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        if (node->token != CMM_TK_DecList) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        if (node->len != 1 && node->len != 3) { REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE); }

        analyze_dec(node->nodes + 0,
                    (struct AnalyCtxDec){.ty        = args.ty,
                                         .where     = args.where,
                                         .fill_into = args.fill_into,
                                         .offset    = args.offset});

        if (node->len == 1) {
            // Dec
            RETURN_WITH_TRACE(CMM_SE_OK);
        } else if (node->len == 3) {
            // Dec COMMA DecList
            node = node->nodes + 2;
        }
    }
}

/// Dec: VarDec | VarDec ASSIGNOP Exp
enum CMM_SEMANTIC analyze_dec(CMM_AST_NODE* node, struct AnalyCtxDec args) {
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_Dec) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    if (node->len != 1 && node->len != 3) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    CMM_AST_NODE* vardec = node->nodes + 0;
    ANALYZE_EXPECT_OK(analyze_var_dec(
        vardec, (struct AnalyCtxVarDec){.where = args.where, .ty = args.ty}));

    args.fill_into[*args.offset] = vardec->context.data.type;
    *args.offset                 = *args.offset + 1;

    if (node->len == 3) {
        if (args.where == INSIDE_A_STRUCT) {
            REPORT_AND_RETURN(CMM_SE_BAD_STRUCT_DOMAIN);
        }
        CMM_AST_NODE* exp = node->nodes + 2;
        analyze_exp(exp, (struct AnalyCtxExp){._void = 0});
        // 赋值语句的类型检查
        if (!cmm_ty_fitable(exp->context.data.type, args.ty)) {
            REPORT_AND_RETURN(CMM_SE_ASSIGN_TYPE_ERROR);
        }
    }

    RETURN_WITH_TRACE(CMM_SE_OK);
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
enum CMM_SEMANTIC analyze_exp(CMM_AST_NODE* node, struct AnalyCtxExp args) {
    FUNCTION_TRACE;
    if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    if (node->token != CMM_TK_Exp) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);

    node->context.kind       = CMM_AST_KIND_TYPE;
    node->context.data.type  = cmm_ty_make_error();
    node->context.value_kind = RVALUE;

    CMM_AST_NODE* a = node->nodes + 0;

    if (a->token == CMM_TK_Exp) {
        analyze_exp(a, args);

        CMM_AST_NODE* op     = node->nodes + 1;
        CMM_AST_NODE* b      = node->nodes + 2;
        CMM_SEM_TYPE  type_a = a->context.data.type;

        /// struct.field
        if (op->token == CMM_TK_DOT) {
            node->context.value_kind = LVALUE;
            if (type_a.kind == CMM_ERROR_TYPE) { RETURN_WITH_TRACE(CMM_SE_OK); }
#ifdef CMM_DEBUG_FLAGTRACE
            printf("trying get %s.%s (size = %d, {",
                   type_a.name,
                   b->data.val_ident,
                   type_a.size);
            fflush(stdout);
            for (int i = 0; i < type_a.size; i++) {
                printf(" %s: %s,", type_a.inner[i].bind, type_a.inner[i].name);
                fflush(stdout);
            }
            printf(" }\n");
            fflush(stdout);
#endif
            if (type_a.kind != CMM_PROD_TYPE) {
                REPORT_AND_RETURN(CMM_SE_BAD_STRUCT_ACCESS);
            }
            CMM_SEM_TYPE* ty = cmm_ty_field_of_struct(type_a, b->data.val_ident);
            if (ty == NULL) { REPORT_AND_RETURN(CMM_SE_UNDEFINED_STRUCT_DOMAIN); }
            node->context.data.type  = *ty;
            node->context.value_kind = LVALUE;
            RETURN_WITH_TRACE(CMM_SE_OK);
        }

        analyze_exp(b, args);
        CMM_SEM_TYPE type_b = b->context.data.type;

        switch (op->token) {
            case CMM_TK_ASSIGNOP:
                if (!cmm_ty_fitable(type_a, type_b)) {
                    REPORT_AND_RETURN(CMM_SE_ASSIGN_TYPE_ERROR);
                }
                if (a->context.value_kind == RVALUE) {
                    REPORT_AND_RETURN(CMM_SE_ASSIGN_TO_RVALUE);
                }
                node->context.data.type = type_a;
                break;
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
            /// array[idx]
            case CMM_TK_LB:
                node->context.value_kind = LVALUE;
                if (type_a.kind == CMM_ERROR_TYPE) { RETURN_WITH_TRACE(CMM_SE_OK); }
                if (type_a.kind != CMM_ARRAY_TYPE) {
                    REPORT_AND_RETURN(CMM_SE_BAD_ARRAY_ACCESS);
                }
                if (type_b.kind == CMM_ERROR_TYPE || (type_b.kind == CMM_PRIMITIVE_TYPE &&
                                                      strcmp(type_b.name, "int") == 0)) {
                    node->context.data.type = *type_a.inner;
                } else {
                    REPORT_AND_RETURN(CMM_SE_BAD_ARRAY_INDEX);
                }
                break;
            default: REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        }
    } else if (a->token == CMM_TK_ID) {
#ifdef CMM_DEBUG_FLAGTRACE
        printf("[search] will find %s\n", a->data.val_ident);
#endif
        const SemanticContext* a_def = find_defination(a->data.val_ident);
        if (a_def == NULL) {
            if (node->len == 1) {
                REPORT_AND_RETURN(CMM_SE_UNDEFINED_VARIABLE);
            } else {
                REPORT_AND_RETURN(CMM_SE_UNDEFINED_FUNCTION);
            }
        } else {
            node->context.data.type = a_def->ty;
        }

        if (node->len == 1) {
            node->context.value_kind = LVALUE;
            RETURN_WITH_TRACE(CMM_SE_OK);
        }

        if (a_def->ty.kind != CMM_FUNCTION_TYPE) {
            REPORT_AND_RETURN(CMM_SE_BAD_FUNCTION_CALL);
        }

        node->context.data.type = a_def->ty.inner[a_def->ty.size - 1];

        if (node->len == 3) {
            if (a_def->ty.size != 1) { REPORT_AND_RETURN(CMM_SE_ARGS_NOT_MATCH); }
        } else if (node->len == 4) {
            CMM_AST_NODE* arg_node = node->nodes + 2;
            analyze_args(arg_node, (struct AnalyCtxArgs){.calling = a_def->ty});
        }
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
        // 括号
        CMM_AST_NODE* b = node->nodes + 1;
        analyze_exp(b, args);
        CMM_SEM_TYPE type_b      = b->context.data.type;
        node->context.data.type  = type_b;
        node->context.value_kind = b->context.value_kind;
    } else if (a->token == CMM_TK_INT) {
        node->context.data.type = cmm_ty_make_primitive("int");
    } else if (a->token == CMM_TK_FLOAT) {
        node->context.data.type = cmm_ty_make_primitive("float");
    } else {
        REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
    }

    RETURN_WITH_TRACE(CMM_SE_OK);
}


// Args: Exp COMMA Args
//     | Exp
enum CMM_SEMANTIC analyze_args(CMM_AST_NODE* root, struct AnalyCtxArgs args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;

    // args.calling.size >= 2
    int return_type_idx = args.calling.size - 1;
    int tail_idx        = args.calling.size - 2;

    for (int i = 0;; i++) {
        if (node == NULL) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        if (node->token != CMM_TK_Args) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        if (node->len != 1 && node->len != 3) REPORT_AND_RETURN(CMM_SE_BAD_AST_TREE);
        if (i == return_type_idx) { REPORT_AND_RETURN(CMM_SE_ARGS_NOT_MATCH); }

        CMM_AST_NODE* param = node->nodes + 0;

        analyze_exp(param, (struct AnalyCtxExp){._void = 0});

        if (!cmm_ty_fitable(args.calling.inner[i], param->context.data.type)) {
            REPORT_AND_RETURN(CMM_SE_ARGS_NOT_MATCH);
        }

        if (node->len == 1) {
            if (i != tail_idx) { REPORT_AND_RETURN(CMM_SE_ARGS_NOT_MATCH); }
            break;
        } else if (node->len == 3) {
            node = node->nodes + 2;
        }
    }

    root->context.kind      = CMM_AST_KIND_TYPE;
    root->context.data.type = args.calling.inner[return_type_idx];
    RETURN_WITH_TRACE(CMM_SE_OK);
}

#pragma endregion