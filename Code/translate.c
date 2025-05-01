#include "translate.h"
#include "hashmap.h"
#include "predefines.h"
#include "syndef.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#pragma region Definations

typedef void tres;

#ifdef CMM_DEBUG_LAB3TRACE
int __trans_trace_spaces = 0;
#    define FUNCTION_TRACE                                                               \
        {                                                                                \
            __trans_trace_spaces++;                                                      \
            for (int i = 0; i < __trans_trace_spaces; i++) printf(" ");                  \
            printf("\033[1;34m  >== %s : %s:%d\n\033[0m", __func__, __FILE__, __LINE__); \
        }
#    define RETURN_WITH_TRACE(x)
{
    __trans_trace_spaces--;
    return x;
}
#else
#    define FUNCTION_TRACE \
        {}
#    define RETURN_WITH_TRACE(x) \
        { return x; }
#endif


typedef struct TransContext {
    int          line;
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
} TransContext;

typedef struct TransScope {
    char*              name;
    struct TransScope* back;
    struct TransScope* data;
    TransContext*      ctx;
} TransScope;

enum VAR_WHERE {
    INSIDE_A_BLOCK,
    INSIDE_A_STRUCT,
};

struct TCtxProgram {
    int _void;
};
struct TCtxExtDefList {
    int _void;
};
struct TCtxExtDef {
    int _void;
};
struct TCtxSpecifier {
    int _void;
};
struct TCtxExtDecList {
    CMM_SEM_TYPE ty;
};
struct TCtxFunDec {
    CMM_SEM_TYPE return_ty;
    int          is_def;
};
struct TCtxCompSt {
    CMM_SEM_TYPE current_fn_ty;
};
struct TCtxVarDec {
    enum VAR_WHERE where;
    CMM_SEM_TYPE   ty;
};
struct TCtxStructSpecifier {
    int _void;
};
struct TCtxOptTag {
    int _void;
};
struct TCtxDefList {
    enum VAR_WHERE where;
    int*           struct_fields_len;
    CMM_SEM_TYPE** struct_fields_types;
};
struct TCtxTag {
    int _void;
};
struct TCtxVarList {
    int*           list_len;
    CMM_SEM_TYPE** arg_types;
};
struct TCtxParamDec {
    CMM_SEM_TYPE* ty;
};
struct TCtxStmtList {
    CMM_SEM_TYPE current_fn_ty;
};
struct TCtxStmt {
    CMM_SEM_TYPE current_fn_ty;
};
struct TCtxExp {
    int _void;
};
struct TCtxDef {
    enum VAR_WHERE where;
    CMM_SEM_TYPE*  fill_into;
    int*           offset;
};
struct TCtxDecList {
    CMM_SEM_TYPE   ty;
    enum VAR_WHERE where;
    CMM_SEM_TYPE*  fill_into;
    int*           offset;
};
struct TCtxDec {
    CMM_SEM_TYPE   ty;
    enum VAR_WHERE where;
    CMM_SEM_TYPE*  fill_into;
    int*           offset;
};
struct TCtxArgs {
    CMM_SEM_TYPE calling;
};

const TransContext* get_trans_defination(const char* name);

tres trans_program(CMM_AST_NODE* node, struct TCtxProgram args);
tres trans_ext_def_list(CMM_AST_NODE* node, struct TCtxExtDefList args);
tres trans_ext_def(CMM_AST_NODE* node, struct TCtxExtDef args);
tres trans_specifier(CMM_AST_NODE* node, struct TCtxSpecifier args);
tres trans_ext_dec_list(CMM_AST_NODE* node, struct TCtxExtDecList args);
tres trans_fun_dec(CMM_AST_NODE* node, struct TCtxFunDec args);
tres trans_comp_st(CMM_AST_NODE* node, struct TCtxCompSt args);
tres trans_var_dec(CMM_AST_NODE* node, struct TCtxVarDec args);
tres trans_struct_specifier(CMM_AST_NODE* node, struct TCtxStructSpecifier args);
tres trans_opt_tag(CMM_AST_NODE* node, struct TCtxOptTag args);
tres trans_def_list(CMM_AST_NODE* node, struct TCtxDefList args);
tres trans_tag(CMM_AST_NODE* node, struct TCtxTag args);
tres trans_var_list(CMM_AST_NODE* node, struct TCtxVarList args);
tres trans_param_dec(CMM_AST_NODE* node, struct TCtxParamDec args);
tres trans_stmt_list(CMM_AST_NODE* node, struct TCtxStmtList args);
tres trans_stmt(CMM_AST_NODE* node, struct TCtxStmt args);
tres trans_exp(CMM_AST_NODE* node, struct TCtxExp args);
tres trans_def(CMM_AST_NODE* node, struct TCtxDef args);
tres trans_dec_list(CMM_AST_NODE* node, struct TCtxDecList args);
tres trans_dec(CMM_AST_NODE* node, struct TCtxDec args);
tres trans_args(CMM_AST_NODE* node, struct TCtxArgs args);
#pragma endregion

#pragma region Global States
/// 一个哈希表，用来存放trans的context
struct hashmap* trans_context    = NULL;
/// 一个链表，用来存放当前scope的名字。越是顶层越在后方
TransScope*     trans_scope      = NULL;
TransScope*     root_trans_scope = NULL;
#pragma endregion


#pragma region Helper Functions

void free_trans_ctx(void* data) {
    // if (data == NULL) return;
    // const TransContext* ctx = data;
    // free(ctx->name);
    (void)data;
}

int trans_ctx_compare(const void* a, const void* b, void* _udata) {
    const TransContext* ua = a;
    const TransContext* ub = b;
    (void)(_udata);
    return strcmp(ua->name, ub->name);
}

uint64_t trans_ctx_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const TransContext* ctx = item;
    return hashmap_sip(ctx->name, strlen(ctx->name), seed0, seed1);
}

/// 释放当前 TransScope 节点，返回它的前驱（或者说后驱）
TransScope* free_trans_scope(TransScope* node) {
    if (node == NULL) { return NULL; }
    TransScope* ret = node->back;
    free(node->name);
    free(node);
    return ret;
}

/// 进入一个新的语义分析作用域
TransScope* __force_enter_trans_scope(const char* name) {
    TransScope* scope = (TransScope*)malloc(sizeof(TransScope));
    scope->name       = cmm_clone_string(name);
    scope->data       = NULL;
    scope->back       = trans_scope;
    trans_scope       = scope;

#ifdef CMM_DEBUG_LAB3TRACE
    printf("\033[1;32mentering scope: %s\033[0m\n", name);
#endif

    return scope;
}

/// 退出当前的语义分析作用域
void __force_exit_trans_scope() {
#ifdef CMM_DEBUG_LAB3TRACE
    printf("\033[1;32mquiting scope: %s\033[0m\n", trans_scope->name);
#endif
    // 释放当前作用域的所有定义
    TransScope* ptr = trans_scope->data;
    while (ptr != NULL) {
        TransContext* deleted = (TransContext*)hashmap_delete(
            trans_context, &(TransContext){.name = ptr->name});

        if (deleted->def == SEM_CTX_DECLARE) {
            cmm_panic("function is declared but not defined");
        }

        if (deleted != NULL) { free_trans_ctx(deleted); }

        ptr = free_trans_scope(ptr);
    }
    trans_scope = free_trans_scope(trans_scope);
}

TransScope* parent_trans_scope(TransScope* scope) {
    return scope == root_trans_scope ? scope : trans_scope->back;
}

/// 是谁蠢到写完了才发现嵌套作用域的要求是Requirement2啊？？
#ifdef LAB2_REQUIREMENT_2
TransScope* enter_trans_scope(const char* name) { return __enter_trans_scope(name); }
void        exit_trans_scope() { __exit_trans_scope(); }
#else
int         __trans_scope_count_ = 0;
TransScope* enter_trans_scope(const char* name) {
    if (__trans_scope_count_ == 0) { __force_enter_trans_scope(name); }
    __trans_scope_count_++;
    return trans_scope;
}
void exit_trans_scope() {
    __trans_scope_count_--;
    if (__trans_scope_count_ == 0) { __force_exit_trans_scope(); }
}
#endif

char* _trans_ctx_finder(const char* name, TransScope* scope) {
    if (scope == NULL) {
        return cmm_clone_string(name);
    } else {
        char* tmp = _trans_ctx_finder(scope->name, scope->back);
        if (name == NULL) { return tmp; }
        char* ret = cmm_concat_string(3, tmp, "::", name);
        free(tmp);
        return ret;
    }
}

/// 在当前作用域下产生一个新的定义或声明
/// @returns 1 成功
/// @returns 0 失败
/// @returns -1 函数不匹配
int gen_trans_context(TransContext arg) {
    char* varname = _trans_ctx_finder(arg.name, trans_scope);

    // 检查是否存在这个def
    TransContext* existed =
        (TransContext*)hashmap_get(trans_context, &(TransContext){.name = varname});

    if (existed != NULL) {
#ifdef CMM_DEBUG_LAB3TRACE
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
            int ret = cmm_ty_eq(existed->ty, arg.ty);
            if (ret == 1 && existed->def == SEM_CTX_DECLARE) { existed->def = arg.def; }
            return ret == 0 ? -1 : 1;
        }
        return 0;
    }

    /// 变量的名字不能和类型一样
    const TransContext* existed_rec = get_trans_defination(arg.name);
    if (existed_rec != NULL && existed_rec->kind == SEM_CTX_TYPE &&
        arg.kind == SEM_CTX_VAR) {
        return 0;
    }

#ifdef CMM_DEBUG_LAB3TRACE
    printf("\033[1;33m[register] %s %s : %s\033[0m\n",
           arg.def == SEM_CTX_DECLARE ? "dec" : "def",
           varname,
           arg.ty.name);
#endif

    TransContext* allo_ctx = (TransContext*)malloc(sizeof(TransContext));
    *allo_ctx              = arg;
    allo_ctx->name         = varname;
    hashmap_set(trans_context, allo_ctx);

    TransScope* def   = (TransScope*)malloc(sizeof(TransScope));
    def->name         = varname;
    def->back         = trans_scope->data;
    def->ctx          = allo_ctx;
    trans_scope->data = def;

    return 1;
}

/// 在当前作用域，和它的上n级，获取一个定义
const TransContext* get_trans_defination(const char* name) {
    const TransContext* ret   = NULL;
    TransScope*         scope = trans_scope;

    while (ret == NULL && scope != NULL) {
        char* varname = _trans_ctx_finder(name, scope);
#ifdef CMM_DEBUG_LAB3TRACE
        printf("[search] finding %s\n", varname);
#endif
        ret = hashmap_get(trans_context, &(TransContext){.name = varname});
        free(varname);
        scope = scope->back;
    }

    if (ret == NULL) {}

#ifdef CMM_DEBUG_LAB3TRACE
    if (ret == NULL) {
        printf("[search] not found\n");
    } else {
        printf("[search] found: ty = %s\n", ret->ty.name);
    }
#endif

    return ret;
}
#pragma endregion

#pragma region Functions

int cmm_trans_code(CMM_AST_NODE* node) {
    // prepare
    trans_context = hashmap_new(sizeof(TransContext),
                                65535,
                                0,
                                0,
                                trans_ctx_hash,
                                trans_ctx_compare,
                                free_trans_ctx,
                                NULL);
    enter_trans_scope("root");
    root_trans_scope = trans_scope;

    /// analyze
    trans_program(node, (struct TCtxProgram){._void = 0});

    // clean
    // hashmap_free(trans_context);
    exit_trans_scope();

    return 0;
}

/// Program: ExtDefList;
tres trans_program(CMM_AST_NODE* node, struct TCtxProgram _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Program) cmm_panic("bad ast tree");
    if (node->len != 1) cmm_panic("bad ast tree");

    trans_ext_def_list(node->nodes, (struct TCtxExtDefList){._void = 0});
}

/// ExtDefList: /* empty */ | ExtDef ExtDefList
tres trans_ext_def_list(CMM_AST_NODE* root, struct TCtxExtDefList _) {
    (void)_;
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;
    // 尾递归展开
    while (true) {
        // 结构性错误直接爆
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_ExtDefList) cmm_panic("bad ast tree");

        if (node->len == 0) {

        } else if (node->len == 2) {
            trans_ext_def(node->nodes + 0, (struct TCtxExtDef){._void = 0});
            // TODO
            node = node->nodes + 1;
        } else {
            cmm_panic("bad ast tree");
        }
    }
}

/// ExtDef: Specifier ExtDecList SEMI
/// | Specifier SEMI
/// | Specifier FunDec CompSt
/// | Specifier FunDec SEMI
/// ;
/// 解析扩展的defination
tres trans_ext_def(CMM_AST_NODE* node, struct TCtxExtDef _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_ExtDef) cmm_panic("bad ast tree");

    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* decl      = node->nodes + 1;

    /// 无论这里有没有出错，我们让它继续往下走
    /// 错误的类型会被置为 error 这个特殊的原型类
    trans_specifier(specifier, (struct TCtxSpecifier){._void = 0});

    CMM_SEM_TYPE spec_ty = specifier->context.data.type;

#ifdef CMM_DEBUG_LAB3TRACE
    printf("type is %s\n", spec_ty.name);
#endif

    if (decl->token == CMM_TK_ExtDecList) {
        /// 变量定义
        trans_ext_dec_list(decl, (struct TCtxExtDecList){.ty = spec_ty});

    } else if (decl->token == CMM_TK_FunDec) {
        /// 还需要分析函数体，所以不关心这里的错误
        /// decl->nodes 是一个 &ID，否则AST错误
        char* fn_name = decl->nodes->data.val_ident;

        int is_def = node->nodes[2].token == CMM_TK_CompSt;

        if (is_def) {
            enter_trans_scope(fn_name);
        } else {
            __force_enter_trans_scope(fn_name);
        }

        trans_fun_dec(decl,
                      (struct TCtxFunDec){
                          .return_ty = spec_ty,
                          .is_def    = is_def,
                      });
        if (is_def) {
            // FunDec CompSt
            trans_comp_st(node->nodes + 2,
                          (struct TCtxCompSt){.current_fn_ty = decl->context.data.type});
        } else {
            // FunDec SEMI
            // just a declare
            // TODO
        }

        if (is_def) {
            exit_trans_scope();
        } else {
            __force_exit_trans_scope();
        }

        // TODO
    } else if (decl->token == CMM_TK_SEMI) {
        /// 在这里声明了一个 Type
        /// 期望在 Specifier 里已经注册了这个 Type
    }

    cmm_panic("bad ast tree");
}

/// ExtDecList: VarDec | VarDec COMMA ExtDecList
tres trans_ext_dec_list(CMM_AST_NODE* root, struct TCtxExtDecList args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;
    while (true) {
        // 结构性错误
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_ExtDecList) cmm_panic("bad ast tree");

        trans_var_dec(node->nodes + 0,
                      (struct TCtxVarDec){
                          .ty    = args.ty,
                          .where = INSIDE_A_BLOCK,
                      });
        if (node->len == 1) {

        } else if (node->len == 3) {
            node = node->nodes + 2;
        } else {
            cmm_panic("bad ast tree");
        }
    }
}


/// Specifier: TYPE | StructSpecifier
tres trans_specifier(CMM_AST_NODE* node, struct TCtxSpecifier _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Specifier) cmm_panic("bad ast tree");
    if (node->len != 1) cmm_panic("bad ast tree");

    CMM_AST_NODE* inner = node->nodes;
    node->context.kind  = CMM_AST_KIND_TYPE;

    if (inner->token == CMM_TK_TYPE) {
        node->context.data.type = cmm_ty_make_primitive(inner->data.val_type);

    } else if (inner->token == CMM_TK_StructSpecifier) {
        trans_struct_specifier(inner, (struct TCtxStructSpecifier){._void = 0});
        node->context.data.type = inner->context.data.type;
    }

    cmm_panic("bad ast tree");
}

/// StructSpecifier: STRUCT OptTag LC DefList RC | STRUCT Tag
tres trans_struct_specifier(CMM_AST_NODE* node, struct TCtxStructSpecifier _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_StructSpecifier) cmm_panic("bad ast tree");

    CMM_AST_NODE* tag = node->nodes + 1;

    if (node->len == 2) {
        (trans_tag(tag, (struct TCtxTag){._void = 0}));
        node->context.kind      = CMM_AST_KIND_TYPE;
        const TransContext* ctx = get_trans_defination(tag->context.data.ident);
        if (ctx == NULL) {
            node->context.data.type = cmm_ty_make_error();
            cmm_panic("CMM_SE_UNDEFINED_STRUCT");
        } else {
            node->context.data.type = ctx->ty;
        }
        /// TODO
    } else if (node->len == 5) {
        //  STRUCT OptTag LC DefList RC
        CMM_AST_NODE* opttag  = node->nodes + 1;
        CMM_AST_NODE* deflist = node->nodes + 3;

        (trans_opt_tag(opttag, (struct TCtxOptTag){._void = 0}));

        // 如果 struct 有名字
        char* name = opttag->context.kind == CMM_AST_KIND_IDENT
                         ? opttag->context.data.ident
                         : gen_unnamed_struct_name();

        // 结构体总是要进入一个 scope 的
        __force_enter_trans_scope(name);

        CMM_SEM_TYPE* inner = NULL;
        int           size  = 0;

        trans_def_list(deflist,
                       (struct TCtxDefList){
                           .where               = INSIDE_A_STRUCT,
                           .struct_fields_len   = &size,
                           .struct_fields_types = &inner,
                       });

        __force_exit_trans_scope();
        CMM_SEM_TYPE ty = cmm_ty_make_struct(name, inner, size);

        /// struct 会被提升到顶层
        TransScope* current_scope = trans_scope;
        trans_scope               = root_trans_scope;
        int ok                    = gen_trans_context((TransContext){
                               .def  = SEM_CTX_DEFINATION,
                               .kind = SEM_CTX_TYPE,
                               .name = name,
                               .ty   = ty,
        });
        trans_scope               = current_scope;
        node->context.data.type   = ty;

        if (!ok) { cmm_panic("CMM_SE_DUPLICATE_STRUCT"); }
    } else {
        cmm_panic("bad ast tree");
    }
}

/// Tag: empty | ID
tres trans_opt_tag(CMM_AST_NODE* node, struct TCtxOptTag _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_OptTag) cmm_panic("bad ast tree");
    if (node->len == 0) {

        if (node->len != 1) cmm_panic("bad ast tree");
        CMM_AST_NODE* tag = node->nodes + 0;
        if (tag->token != CMM_TK_ID) cmm_panic("bad ast tree");

        node->context.kind       = CMM_AST_KIND_IDENT;
        node->context.data.ident = tag->data.val_ident;
    }
}

/// Tag: ID
tres trans_tag(CMM_AST_NODE* node, struct TCtxTag _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Tag) cmm_panic("bad ast tree");
    if (node->len != 1) cmm_panic("bad ast tree");

    CMM_AST_NODE* tag = node->nodes + 0;
    if (tag->token != CMM_TK_ID) cmm_panic("bad ast tree");

    node->context.kind       = CMM_AST_KIND_IDENT;
    node->context.data.ident = tag->data.val_ident;
}

/// VarDec: ID | VarDec LB INT RB
/// TODO
tres trans_var_dec(CMM_AST_NODE* node, struct TCtxVarDec args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_VarDec) cmm_panic("bad ast tree");

    if (node->len == 1) {
        // ID
        node->context.kind       = CMM_AST_KIND_IDENT;
        node->context.data.ident = node->nodes->data.val_ident;
        /// 注册这个变量
        args.ty.bind             = node->context.data.ident;
        int ok                   = gen_trans_context((TransContext){
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
                case INSIDE_A_BLOCK: cmm_panic("CMM_SE_DUPLICATE_VARIABLE_DEFINATION");
                case INSIDE_A_STRUCT: cmm_panic("CMM_SE_BAD_STRUCT_DOMAIN");
            }
        }

    } else if (node->len == 4) {
        // VarDec LB INT RB
        int           array_size = node->nodes[2].data.val_int;
        CMM_SEM_TYPE* ty         = malloc(sizeof(CMM_SEM_TYPE));
        *ty                      = args.ty;
        CMM_AST_NODE* vardec     = node->nodes + 0;
        (trans_var_dec(vardec,
                       (struct TCtxVarDec){.where = args.where,
                                           .ty    = cmm_ty_make_array(ty, array_size)}));

        node->context.kind       = CMM_AST_KIND_IDENT;
        node->context.data.ident = vardec->context.data.ident;
        node->context.data.type  = vardec->context.data.type;
        // TODO
    } else {
        cmm_panic("bad ast tree");
    }
}

/// FunDec: ID LP VarList RP | ID LP RP
/// TODO
tres trans_fun_dec(CMM_AST_NODE* node, struct TCtxFunDec args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_FunDec) cmm_panic("bad ast tree");

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
        trans_var_list(node->nodes + 2,
                       (struct TCtxVarList){
                           .list_len  = &list_len,
                           .arg_types = &inner,
                       });

        inner[list_len++] = args.return_ty;

        fnty = cmm_ty_make_func(inner, list_len);
    } else {
        cmm_panic("bad ast tree");
    }

    node->context.kind       = CMM_AST_KIND_DECLARE;
    node->context.data.type  = fnty;
    node->context.data.ident = id_name;

    // TODO 判断dec有误？
    {
        /// 函数定义在上层作用域
        /// 先修改当前作用域，然后再恢复
        TransScope* current_scope = trans_scope;
        trans_scope               = parent_trans_scope(trans_scope);
        int ret                   = gen_trans_context((TransContext){
                              .line = id->location.line,
                              .def  = args.is_def ? SEM_CTX_DEFINATION : SEM_CTX_DECLARE,
                              .kind = SEM_CTX_VAR,
                              .name = id_name,
                              .ty   = fnty,
        });
        trans_scope               = current_scope;

        if (ret == -1) { cmm_panic("CMM_SE_CONFLICT_FUNCTION_DECLARATION"); }
        if (ret == 0) {
            if (args.is_def) {
                /// 父作用域定义重复，为了继续分析 comst，我们再
                /// push一个defination到子作用域
                gen_trans_context((TransContext){
                    .def  = SEM_CTX_DEFINATION,
                    .kind = SEM_CTX_VAR,
                    .name = id_name,
                    .ty   = fnty,
                });
                cmm_panic("CMM_SE_DUPLICATE_FUNCTION_DEFINATION");
            } else {
                cmm_panic("CMM_SE_CONFLICT_FUNCTION_DECLARATION");
            }
        }
    }
}

/// VarList: ParamDec COMMA VarList | ParamDec
/// TODO
tres trans_var_list(CMM_AST_NODE* root, struct TCtxVarList args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node      = root;
    int           len       = 0;
    /// 都256个参数了不至于这都不够吧？？
    CMM_SEM_TYPE* arg_types = malloc(sizeof(CMM_SEM_TYPE) * 256);
    while (true) {
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_VarList) cmm_panic("bad ast tree");

        // TODO
        if (node->len != 1 && node->len != 3) cmm_panic("bad ast tree");

        trans_param_dec(node->nodes + 0,
                        (struct TCtxParamDec){
                            .ty = arg_types + len,
                        });

        len++;

        if (len >= 255) { printf("panic: too many arguments"); }

        if (node->len == 1) {
            *args.list_len  = len;
            *args.arg_types = arg_types;

        } else {
            node = node->nodes + 2;
        }
    }
}

/// ParamDec: Specifier VarDec
/// TODO
tres trans_param_dec(CMM_AST_NODE* node, struct TCtxParamDec args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_ParamDec) cmm_panic("bad ast tree");
    if (node->len != 2) cmm_panic("bad ast tree");

    // TODO
    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* vardec    = node->nodes + 1;

    trans_specifier(specifier, (struct TCtxSpecifier){._void = 0});

    CMM_SEM_TYPE spec_ty = specifier->context.data.type;

    trans_var_dec(vardec,
                  (struct TCtxVarDec){
                      .ty    = spec_ty,
                      .where = INSIDE_A_BLOCK,
                  });

    const TransContext* ctx = get_trans_defination(vardec->context.data.ident);
    *args.ty                = ctx->ty;
    args.ty->bind           = vardec->context.data.ident;
}

/// CompSt: LC DefList StmtList RC
/// TODO
tres trans_comp_st(CMM_AST_NODE* node, struct TCtxCompSt args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_CompSt) cmm_panic("bad ast tree");

    if (node->len != 4) cmm_panic("bad ast tree");

    // LC DefList StmtList RC
    CMM_AST_NODE* deflist  = node->nodes + 1;
    CMM_AST_NODE* stmtlist = node->nodes + 2;

    enter_trans_scope("&");

    // TODO
    trans_def_list(deflist, (struct TCtxDefList){.where = INSIDE_A_BLOCK});
    trans_stmt_list(stmtlist, (struct TCtxStmtList){.current_fn_ty = args.current_fn_ty});

    exit_trans_scope();
}

/// StmtList: /* empty */ | Stmt StmtList
/// TODO
tres trans_stmt_list(CMM_AST_NODE* root, struct TCtxStmtList args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;
    while (true) {
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_StmtList) cmm_panic("bad ast tree");

        if (node->len == 0) {

        } else if (node->len == 2) {
            // TODO
            // Stmt StmtList
            trans_stmt(node->nodes + 0,
                       (struct TCtxStmt){.current_fn_ty = args.current_fn_ty});

            node = node->nodes + 1;
        } else {
            cmm_panic("bad ast tree");
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
tres trans_stmt(CMM_AST_NODE* node, struct TCtxStmt args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Stmt) cmm_panic("bad ast tree");

    CMM_AST_NODE* first = node->nodes + 0;
    switch (first->token) {
        case CMM_TK_Exp: trans_exp(first, (struct TCtxExp){._void = 0}); break;
        case CMM_TK_CompSt:
            trans_comp_st(first,
                          (struct TCtxCompSt){.current_fn_ty = args.current_fn_ty});
            break;
        case CMM_TK_RETURN: {
            CMM_AST_NODE* exp = node->nodes + 1;
            trans_exp(exp, (struct TCtxExp){._void = 0});
            CMM_SEM_TYPE need = args.current_fn_ty.inner[args.current_fn_ty.size - 1];
            CMM_SEM_TYPE got  = exp->context.data.type;
            if (!cmm_ty_fitable(need, got)) {
#ifdef CMM_DEBUG_LAB3TRACE
                printf("need %s, got %s\n", need.name, got.name);
#endif
                cmm_panic("CMM_SE_RETURN_TYPE_ERROR")
            }
            break;
        }
        case CMM_TK_IF: {
            CMM_AST_NODE* exp = node->nodes + 2;
            trans_exp(exp, (struct TCtxExp){._void = 0});
            trans_stmt(node->nodes + 4,
                       (struct TCtxStmt){.current_fn_ty = args.current_fn_ty});
            if (node->len == 7) {
                // IF LP Exp RP Stmt ELSE Stmt
                trans_stmt(node->nodes + 6,
                           (struct TCtxStmt){.current_fn_ty = args.current_fn_ty});
            }
            CMM_SEM_TYPE exp_ty = exp->context.data.type;
            if (exp_ty.kind != CMM_ERROR_TYPE && strcmp(exp_ty.name, "int") != 0) {
                cmm_panic("CMM_SE_OPERAND_TYPE_ERROR");
            }
            break;
        }
        case CMM_TK_WHILE: {
            CMM_AST_NODE* exp = node->nodes + 2;
            trans_exp(exp, (struct TCtxExp){._void = 0});
            trans_stmt(node->nodes + 4,
                       (struct TCtxStmt){.current_fn_ty = args.current_fn_ty});
            CMM_SEM_TYPE exp_ty = exp->context.data.type;
            if (exp_ty.kind != CMM_ERROR_TYPE && strcmp(exp_ty.name, "int") != 0) {
                cmm_panic("CMM_SE_OPERAND_TYPE_ERROR");
            }
            break;
        }
        default: {
            cmm_panic("bad ast tree");
            break;
        }
    }
}

/// DefList: /* empty */ | Def DefList
/// TODO
tres trans_def_list(CMM_AST_NODE* root, struct TCtxDefList args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;

    CMM_SEM_TYPE* fields     = malloc(sizeof(CMM_SEM_TYPE) * 500);
    int           fileds_len = 0;

    while (true) {
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_DefList) cmm_panic("bad ast tree");

        if (node->len == 0) {
            if (args.where == INSIDE_A_STRUCT) {
                *args.struct_fields_len   = fileds_len;
                *args.struct_fields_types = fields;
            } else {
                free(fields);
            }

        } else if (node->len == 2) {
            // TODO
            // Def DefList
            trans_def(
                node->nodes + 0,
                (struct TCtxDef){
                    .where = args.where, .fill_into = fields, .offset = &fileds_len});

            node = node->nodes + 1;
        } else {
            cmm_panic("bad ast tree");
        }
    }
}

/// Def: Specifier DecList SEMI
/// TODO
tres trans_def(CMM_AST_NODE* node, struct TCtxDef args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Def) cmm_panic("bad ast tree");

    if (node->len != 3) cmm_panic("bad ast tree");

    // Specifier DecList SEMI
    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* declist   = node->nodes + 1;

    trans_specifier(specifier, (struct TCtxSpecifier){._void = 0});

    CMM_SEM_TYPE spec_ty = specifier->context.data.type;

    trans_dec_list(declist,
                   (struct TCtxDecList){.ty        = spec_ty,
                                        .where     = args.where,
                                        .fill_into = args.fill_into,
                                        .offset    = args.offset});
}

/// DecList: Dec | Dec COMMA DecList
/// TODO
tres trans_dec_list(CMM_AST_NODE* root, struct TCtxDecList args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;
    while (true) {
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_DecList) cmm_panic("bad ast tree");
        if (node->len != 1 && node->len != 3) { cmm_panic("bad ast tree"); }

        trans_dec(node->nodes + 0,
                  (struct TCtxDec){.ty        = args.ty,
                                   .where     = args.where,
                                   .fill_into = args.fill_into,
                                   .offset    = args.offset});

        if (node->len == 1) {
            // Dec

        } else if (node->len == 3) {
            // Dec COMMA DecList
            node = node->nodes + 2;
        }
    }
}

/// Dec: VarDec | VarDec ASSIGNOP Exp
tres trans_dec(CMM_AST_NODE* node, struct TCtxDec args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Dec) cmm_panic("bad ast tree");

    if (node->len != 1 && node->len != 3) cmm_panic("bad ast tree");

    CMM_AST_NODE* vardec = node->nodes + 0;
    (trans_var_dec(vardec, (struct TCtxVarDec){.where = args.where, .ty = args.ty}));

    args.fill_into[*args.offset] = vardec->context.data.type;
    *args.offset                 = *args.offset + 1;

    if (node->len == 3) {
        if (args.where == INSIDE_A_STRUCT) { cmm_panic("CMM_SE_BAD_STRUCT_DOMAIN"); }
        CMM_AST_NODE* exp = node->nodes + 2;
        trans_exp(exp, (struct TCtxExp){._void = 0});
        // 赋值语句的类型检查
        if (!cmm_ty_fitable(exp->context.data.type, args.ty)) {
            cmm_panic("CMM_SE_ASSIGN_TYPE_ERROR");
        }
    }
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
tres trans_exp(CMM_AST_NODE* node, struct TCtxExp args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Exp) cmm_panic("bad ast tree");

    node->context.kind       = CMM_AST_KIND_TYPE;
    node->context.data.type  = cmm_ty_make_error();
    node->context.value_kind = RVALUE;

    CMM_AST_NODE* a = node->nodes + 0;

    if (a->token == CMM_TK_Exp) {
        trans_exp(a, args);

        CMM_AST_NODE* op     = node->nodes + 1;
        CMM_AST_NODE* b      = node->nodes + 2;
        CMM_SEM_TYPE  type_a = a->context.data.type;

        /// struct.field
        if (op->token == CMM_TK_DOT) {
            node->context.value_kind = LVALUE;
            trans_exp(b, args);
            CMM_SEM_TYPE type_b = b->context.data.type;

            switch (op->token) {
                case CMM_TK_ASSIGNOP: node->context.data.type = type_a; break;
                case CMM_TK_AND:
                case CMM_TK_OR: node->context.data.type = type_b; break;
                case CMM_TK_RELOP:
                    node->context.data.type = cmm_ty_make_primitive("int");
                    break;
                case CMM_TK_PLUS:
                case CMM_TK_MINUS:
                case CMM_TK_STAR:
                case CMM_TK_DIV: node->context.data.type = type_b; break;
                /// array[idx]
                case CMM_TK_LB:
                    node->context.value_kind = LVALUE;
                    node->context.data.type  = *type_a.inner;
                    break;
                default: cmm_panic("bad ast tree");
            }
        } else if (a->token == CMM_TK_ID) {
#ifdef CMM_DEBUG_LAB3TRACE
            printf("[search] will find %s\n", a->data.val_ident);
#endif
            const TransContext* a_def = get_trans_defination(a->data.val_ident);
            if (a_def == NULL) {
                if (node->len == 1) {
                    cmm_panic("CMM_SE_UNDEFINED_VARIABLE");
                } else {
                    cmm_panic("CMM_SE_UNDEFINED_FUNCTION");
                }
            } else {
                node->context.data.type = a_def->ty;
            }

            if (node->len == 1) { node->context.value_kind = LVALUE; }

            if (a_def->ty.kind != CMM_FUNCTION_TYPE) {
                cmm_panic("CMM_SE_BAD_FUNCTION_CALL");
            }

            node->context.data.type = a_def->ty.inner[a_def->ty.size - 1];

            if (node->len == 3) {
                if (a_def->ty.size != 1) { cmm_panic("CMM_SE_ARGS_NOT_MATCH"); }
            } else if (node->len == 4) {
                CMM_AST_NODE* arg_node = node->nodes + 2;
                trans_args(arg_node, (struct TCtxArgs){.calling = a_def->ty});
            }
        } else if (a->token == CMM_TK_MINUS) {
            CMM_AST_NODE* b = node->nodes + 1;
            trans_exp(b, args);
            CMM_SEM_TYPE type_b     = b->context.data.type;
            node->context.data.type = type_b;
        } else if (a->token == CMM_TK_NOT) {
            CMM_AST_NODE* b = node->nodes + 1;
            trans_exp(b, args);
            CMM_SEM_TYPE type_b     = b->context.data.type;
            node->context.data.type = type_b;
        } else if (a->token == CMM_TK_LP) {
            // 括号
            CMM_AST_NODE* b = node->nodes + 1;
            trans_exp(b, args);
            CMM_SEM_TYPE type_b      = b->context.data.type;
            node->context.data.type  = type_b;
            node->context.value_kind = b->context.value_kind;
        } else if (a->token == CMM_TK_INT) {
            node->context.data.type = cmm_ty_make_primitive("int");
        } else if (a->token == CMM_TK_FLOAT) {
            node->context.data.type = cmm_ty_make_primitive("float");
        } else {
            cmm_panic("bad ast tree");
        }
    }
}


// Args: Exp COMMA Args
//     | Exp
tres trans_args(CMM_AST_NODE* root, struct TCtxArgs args) {
    FUNCTION_TRACE;
    CMM_AST_NODE* node = root;

    // args.calling.size >= 2
    int return_type_idx = args.calling.size - 1;
    int tail_idx        = args.calling.size - 2;

    for (int i = 0;; i++) {
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_Args) cmm_panic("bad ast tree");
        if (node->len != 1 && node->len != 3) cmm_panic("bad ast tree");
        if (i == return_type_idx) { cmm_panic("CMM_SE_ARGS_NOT_MATCH"); }

        CMM_AST_NODE* param = node->nodes + 0;

        trans_exp(param, (struct TCtxExp){._void = 0});

        if (!cmm_ty_fitable(args.calling.inner[i], param->context.data.type)) {
            cmm_panic("CMM_SE_ARGS_NOT_MATCH");
        }

        if (node->len == 1) {
            if (i != tail_idx) { cmm_panic("CMM_SE_ARGS_NOT_MATCH"); }
            break;
        } else if (node->len == 3) {
            node = node->nodes + 2;
        }
    }

    root->context.kind      = CMM_AST_KIND_TYPE;
    root->context.data.type = args.calling.inner[return_type_idx];
}

#pragma endregion