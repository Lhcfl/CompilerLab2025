#include "translate.h"
#include "hashmap.h"
#include "ir.h"
#include "predefines.h"
#include "syndef.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#pragma region Definations

typedef void* tret;

#undef FUNCTION_TRACE
#undef RETURN_WITH_TRACE

#ifdef CMM_DEBUG_LAB3TRACE
int __trans_trace_spaces = 0;
#    define FUNCTION_TRACE                                                         \
        {                                                                          \
            __trans_trace_spaces++;                                                \
            for (int i = 0; i < __trans_trace_spaces; i++) printf(" ");            \
            printf("\033[1;34m  >== %s : %s:%d : ", __func__, __FILE__, __LINE__); \
            cmm_debug_show_node_info(node, 2);                                     \
            printf("\n\033[0m");                                                   \
        }
#    define RETURN_WITH_TRACE(x)    \
        {                           \
            __trans_trace_spaces--; \
            return x;               \
        }
#else
#    define FUNCTION_TRACE \
        {}
#    define RETURN_WITH_TRACE(x) \
        { return x; }
#endif


typedef struct TransDef {
    int          line;
    char*        name;
    CMM_SEM_TYPE ty;
    enum SemContextKind {
        TRANS_CTX_TYPE,
        TRANS_CTX_VAR,
    } kind;
    CMM_IR_VAR ir_var;
} TransDef;

typedef struct TransScope {
    char*              name;
    struct TransScope* back;
    struct TransScope* data;
    TransDef*          ctx;
} TransScope;

enum VAR_WHERE {
    INSIDE_A_BLOCK,
    INSIDE_A_STRUCT,
};

struct TargProgram {
    int _void;
};
struct TargExtDefList {
    int _void;
};
struct TargExtDef {
    int _void;
};
struct TargSpecifier {
    int _void;
};
struct TargExtDecList {
    CMM_SEM_TYPE ty;
};
struct TargFunDec {
    CMM_SEM_TYPE return_ty;
    int          is_def;
};
struct TargCompSt {
    CMM_SEM_TYPE current_fn_ty;
};
struct TargVarDec {
    enum VAR_WHERE where;
    CMM_SEM_TYPE   ty;
};
struct TargStructSpecifier {
    int _void;
};
struct TargOptTag {
    int _void;
};
struct TargDefList {
    enum VAR_WHERE where;
    int*           struct_fields_len;
    CMM_SEM_TYPE** struct_fields_types;
};
struct TargTag {
    int _void;
};
struct TargVarList {
    int*           list_len;
    CMM_SEM_TYPE** arg_types;
};
struct TargParamDec {
    CMM_SEM_TYPE* ty;
};
struct TargStmtList {
    CMM_SEM_TYPE current_fn_ty;
};
struct TargStmt {
    CMM_SEM_TYPE current_fn_ty;
};
struct TargExp {
    int _void;
};
struct TargDef {
    enum VAR_WHERE where;
    CMM_SEM_TYPE*  fill_into;
    int*           offset;
};
struct TargDecList {
    CMM_SEM_TYPE   ty;
    enum VAR_WHERE where;
    CMM_SEM_TYPE*  fill_into;
    int*           offset;
};
struct TargDec {
    CMM_SEM_TYPE   ty;
    enum VAR_WHERE where;
    CMM_SEM_TYPE*  fill_into;
    int*           offset;
};
struct TargArgs {
    CMM_SEM_TYPE calling;
};

typedef struct TretOptTag {
    char* name;
} TretOptTag;

typedef struct TretVarDec {
    char*        varname;
    CMM_IR_VAR   bind;
    CMM_SEM_TYPE type;
} TretVarDec;

const TransDef* get_trans_defination(const char* name);

tret       trans_program(CMM_AST_NODE* node, struct TargProgram args);
tret       trans_ext_def_list(CMM_AST_NODE* node, struct TargExtDefList args);
tret       trans_ext_def(CMM_AST_NODE* node, struct TargExtDef args);
tret       trans_specifier(CMM_AST_NODE* node, struct TargSpecifier args);
tret       trans_ext_dec_list(CMM_AST_NODE* node, struct TargExtDecList args);
tret       trans_fun_dec(CMM_AST_NODE* node, struct TargFunDec args);
tret       trans_comp_st(CMM_AST_NODE* node, struct TargCompSt args);
TretVarDec trans_var_dec(CMM_AST_NODE* node, struct TargVarDec args);
tret       trans_struct_specifier(CMM_AST_NODE* node, struct TargStructSpecifier args);
TretOptTag trans_opt_tag(CMM_AST_NODE* node, struct TargOptTag args);
tret       trans_def_list(CMM_AST_NODE* node, struct TargDefList args);
tret       trans_tag(CMM_AST_NODE* node, struct TargTag args);
tret       trans_var_list(CMM_AST_NODE* node, struct TargVarList args);
tret       trans_param_dec(CMM_AST_NODE* node, struct TargParamDec args);
tret       trans_stmt_list(CMM_AST_NODE* node, struct TargStmtList args);
tret       trans_stmt(CMM_AST_NODE* node, struct TargStmt args);
tret       trans_exp(CMM_AST_NODE* node, struct TargExp args);
tret       trans_def(CMM_AST_NODE* node, struct TargDef args);
tret       trans_dec_list(CMM_AST_NODE* node, struct TargDecList args);
tret       trans_dec(CMM_AST_NODE* node, struct TargDec args);
tret       trans_args(CMM_AST_NODE* node, struct TargArgs args);
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
    // const TransDef* ctx = data;
    // free(ctx->name);
    (void)data;
}

int trans_ctx_compare(const void* a, const void* b, void* _udata) {
    const TransDef* ua = a;
    const TransDef* ub = b;
    (void)(_udata);
    return strcmp(ua->name, ub->name);
}

uint64_t trans_ctx_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const TransDef* ctx = item;
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
        TransDef* deleted =
            (TransDef*)hashmap_delete(trans_context, &(TransDef){.name = ptr->name});

        // if (deleted->def == TRANS_CTX_DECLARE) {
        //     cmm_panic("function is declared but not defined");
        // }

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
void push_trans_def(TransDef arg) {
    char* varname = _trans_ctx_finder(arg.name, trans_scope);

    // 检查是否存在这个def
    TransDef* existed =
        (TransDef*)hashmap_get(trans_context, &(TransDef){.name = varname});

    if (existed != NULL) cmm_panic("defined again %s : %s", varname, arg.ty.name);

    /// 变量的名字不能和类型一样
    const TransDef* existed_rec = get_trans_defination(arg.name);
    if (existed_rec != NULL && existed_rec->kind == TRANS_CTX_TYPE &&
        arg.kind == TRANS_CTX_VAR)
        cmm_panic("var name is same as type name %s : %s", varname, existed_rec->ty.name);

#ifdef CMM_DEBUG_LAB3TRACE
    printf("\033[1;33m[defined] %s : %s\033[0m\n", varname, arg.ty.name);
#endif

    TransDef* allo_ctx = (TransDef*)malloc(sizeof(TransDef));
    *allo_ctx          = arg;
    allo_ctx->name     = varname;
    hashmap_set(trans_context, allo_ctx);

    TransScope* def   = (TransScope*)malloc(sizeof(TransScope));
    def->name         = varname;
    def->back         = trans_scope->data;
    def->ctx          = allo_ctx;
    trans_scope->data = def;
}

/// 在当前作用域，和它的上n级，获取一个定义
const TransDef* get_trans_defination(const char* name) {
    const TransDef* ret   = NULL;
    TransScope*     scope = trans_scope;

    while (ret == NULL && scope != NULL) {
        char* varname = _trans_ctx_finder(name, scope);
        // #ifdef CMM_DEBUG_LAB3TRACE
        //         printf("[search] finding %s\n", varname);
        // #endif
        ret           = hashmap_get(trans_context, &(TransDef){.name = varname});
        free(varname);
        scope = scope->back;
    }

    // #ifdef CMM_DEBUG_LAB3TRACE
    //     if (ret == NULL) {
    //         cmm_debug(COLOR_CYAN, "[search] %s not found\n", name);
    //     } else {
    //         cmm_debug(COLOR_CYAN, "[search] found: %s : %s\n", name, ret->ty.name);
    //     }
    // #endif

    return ret;
}
#pragma endregion

#pragma region Functions

int cmm_trans_code(CMM_AST_NODE* node) {
    // prepare
    trans_context = hashmap_new(sizeof(TransDef),
                                65535,
                                0,
                                0,
                                trans_ctx_hash,
                                trans_ctx_compare,
                                free_trans_ctx,
                                NULL);

    enter_trans_scope("root");
    root_trans_scope = trans_scope;

    // read: () -> int
    push_trans_def((TransDef){
        .kind = TRANS_CTX_VAR,
        .name = cmm_clone_string("read"),
        .ty   = cmm_create_function_type(1, "int"),
        .line = 0,
    });

    // write: int -> int
    push_trans_def((TransDef){
        .kind = TRANS_CTX_VAR,
        .name = cmm_clone_string("write"),
        .ty   = cmm_create_function_type(2, "int", "int"),
        .line = 0,
    });

    /// analyze
    trans_program(node, (struct TargProgram){._void = 0});

    // clean
    // hashmap_free(trans_context);
    exit_trans_scope();

    return 0;
}

/// Program: ExtDefList;
tret trans_program(CMM_AST_NODE* node, struct TargProgram _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Program) cmm_panic("bad ast tree");
    if (node->len != 1) cmm_panic("bad ast tree");

    trans_ext_def_list(node->nodes, (struct TargExtDefList){._void = 0});

    RETURN_WITH_TRACE(0);
}

/// ExtDefList: /* empty */ | ExtDef ExtDefList
tret trans_ext_def_list(CMM_AST_NODE* root, struct TargExtDefList _) {
    (void)_;
    CMM_AST_NODE* node = root;
    FUNCTION_TRACE;
    // 尾递归展开
    while (true) {
        // 结构性错误直接爆
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_ExtDefList) cmm_panic("bad ast tree");

        if (node->len == 0) {
            RETURN_WITH_TRACE(0);
        } else if (node->len == 2) {
            trans_ext_def(node->nodes + 0, (struct TargExtDef){._void = 0});
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
tret trans_ext_def(CMM_AST_NODE* node, struct TargExtDef _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_ExtDef) cmm_panic("bad ast tree");

    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* decl      = node->nodes + 1;

    /// 无论这里有没有出错，我们让它继续往下走
    /// 错误的类型会被置为 error 这个特殊的原型类
    trans_specifier(specifier, (struct TargSpecifier){._void = 0});

    CMM_SEM_TYPE spec_ty = specifier->trans.type;

    if (decl->token == CMM_TK_ExtDecList) {
        /// 变量定义
        trans_ext_dec_list(decl, (struct TargExtDecList){.ty = spec_ty});
        RETURN_WITH_TRACE(0);
    } else if (decl->token == CMM_TK_FunDec) {
        /// 还需要分析函数体，所以不关心这里的错误
        /// decl->nodes 是一个 &ID，否则AST错误
        char* fn_name = decl->nodes->data.val_ident;

        int is_def = node->nodes[2].token == CMM_TK_CompSt;

        if (is_def) {
            enter_trans_scope(fn_name);
        } else {
            cmm_panic("lab3 does not support function declaration");
        }

        trans_fun_dec(decl,
                      (struct TargFunDec){
                          .return_ty = spec_ty,
                          .is_def    = is_def,
                      });
        if (is_def) {
            // FunDec CompSt
            trans_comp_st(node->nodes + 2,
                          (struct TargCompSt){.current_fn_ty = decl->trans.type});
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

        RETURN_WITH_TRACE(0);
    } else if (decl->token == CMM_TK_SEMI) {
        /// 在这里声明了一个 Type
        /// 期望在 Specifier 里已经注册了这个 Type
        RETURN_WITH_TRACE(0);
    }

    cmm_panic("bad ast tree");
}

/// ExtDecList: VarDec | VarDec COMMA ExtDecList
tret trans_ext_dec_list(CMM_AST_NODE* root, struct TargExtDecList args) {
    CMM_AST_NODE* node = root;
    FUNCTION_TRACE;
    while (true) {
        // 结构性错误
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_ExtDecList) cmm_panic("bad ast tree");

        trans_var_dec(node->nodes + 0,
                      (struct TargVarDec){
                          .ty    = args.ty,
                          .where = INSIDE_A_BLOCK,
                      });

        if (node->len == 1) {
            RETURN_WITH_TRACE(0);
        } else if (node->len == 3) {
            node = node->nodes + 2;
        } else {
            cmm_panic("bad ast tree");
        }
    }
}


/// Specifier: TYPE | StructSpecifier
tret trans_specifier(CMM_AST_NODE* node, struct TargSpecifier _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Specifier) cmm_panic("bad ast tree");
    if (node->len != 1) cmm_panic("bad ast tree");

    CMM_AST_NODE* inner = node->nodes;

    if (inner->token == CMM_TK_TYPE) {
        node->trans.type = cmm_ty_make_primitive(inner->data.val_type);
        RETURN_WITH_TRACE(0);
    } else if (inner->token == CMM_TK_StructSpecifier) {
        trans_struct_specifier(inner, (struct TargStructSpecifier){._void = 0});
        node->trans.type = inner->trans.type;
        RETURN_WITH_TRACE(0);
    }

    cmm_panic("bad ast tree");
}

/// StructSpecifier: STRUCT OptTag LC DefList RC | STRUCT Tag
tret trans_struct_specifier(CMM_AST_NODE* node, struct TargStructSpecifier _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_StructSpecifier) cmm_panic("bad ast tree");

    CMM_AST_NODE* tag = node->nodes + 1;

    if (node->len == 2) {
        trans_tag(tag, (struct TargTag){._void = 0});
        const TransDef* ctx = get_trans_defination(tag->trans.ident);
        if (ctx == NULL) {
            node->trans.type = cmm_ty_make_error();
            cmm_panic("CMM_SE_UNDEFINED_STRUCT");
        } else {
            node->trans.type = ctx->ty;
        }
        /// TODO
    } else if (node->len == 5) {
        //  STRUCT OptTag LC DefList RC
        CMM_AST_NODE* opttag  = node->nodes + 1;
        CMM_AST_NODE* deflist = node->nodes + 3;

        TretOptTag res = trans_opt_tag(opttag, (struct TargOptTag){._void = 0});

        // 如果 struct 有名字
        char* name = res.name ? res.name : gen_unnamed_struct_name();

        // 结构体总是要进入一个 scope 的
        __force_enter_trans_scope(name);

        CMM_SEM_TYPE* inner = NULL;
        int           size  = 0;

        trans_def_list(deflist,
                       (struct TargDefList){
                           .where               = INSIDE_A_STRUCT,
                           .struct_fields_len   = &size,
                           .struct_fields_types = &inner,
                       });

        __force_exit_trans_scope();
        CMM_SEM_TYPE ty = cmm_ty_make_struct(name, inner, size);

        /// struct 会被提升到顶层
        TransScope* current_scope = trans_scope;
        trans_scope               = root_trans_scope;

        push_trans_def((TransDef){
            .kind = TRANS_CTX_TYPE,
            .name = name,
            .ty   = ty,
        });

        trans_scope      = current_scope;
        node->trans.type = ty;
    } else {
        cmm_panic("bad ast tree");
    }

    RETURN_WITH_TRACE(0);
}

/// Tag: empty | ID
TretOptTag trans_opt_tag(CMM_AST_NODE* node, struct TargOptTag _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_OptTag) cmm_panic("bad ast tree");
    if (node->len == 0) {
        RETURN_WITH_TRACE(((TretOptTag){
            .name = NULL,
        }));
    }
    if (node->len == 1) {
        CMM_AST_NODE* tag = node->nodes + 0;
        if (tag->token != CMM_TK_ID) cmm_panic("bad ast tree");
        node->trans.ident = tag->data.val_ident;
        RETURN_WITH_TRACE(((TretOptTag){
            .name = tag->data.val_ident,
        }));
    }
    cmm_panic("bad ast tree");
}

/// Tag: ID
tret trans_tag(CMM_AST_NODE* node, struct TargTag _) {
    (void)(_);
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Tag) cmm_panic("bad ast tree");
    if (node->len != 1) cmm_panic("bad ast tree");

    CMM_AST_NODE* tag = node->nodes + 0;
    if (tag->token != CMM_TK_ID) cmm_panic("bad ast tree");

    node->trans.ident = tag->data.val_ident;

    RETURN_WITH_TRACE(0);
}

/// VarDec: ID | VarDec LB INT RB
/// TODO
TretVarDec trans_var_dec(CMM_AST_NODE* node, struct TargVarDec args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_VarDec) cmm_panic("bad ast tree");

    if (node->len == 1) {
        // ID
        char* id          = node->nodes->data.val_ident;
        /// 注册这个变量
        args.ty.bind      = id;
        CMM_IR_VAR ir_var = ir_new_var();

        push_trans_def((TransDef){
            .kind   = TRANS_CTX_VAR,
            .name   = id,
            .ty     = args.ty,
            .ir_var = ir_var,
        });

        RETURN_WITH_TRACE(((TretVarDec){
            .varname = id,
            .bind    = ir_var,
            .type    = args.ty,
        }));
    } else if (node->len == 4) {
        // VarDec LB INT RB
        int           array_size = node->nodes[2].data.val_int;
        CMM_SEM_TYPE* ty         = malloc(sizeof(CMM_SEM_TYPE));
        *ty                      = args.ty;
        CMM_AST_NODE* vardec     = node->nodes + 0;

        RETURN_WITH_TRACE(trans_var_dec(vardec,
                                        (struct TargVarDec){
                                            .where = args.where,
                                            .ty    = cmm_ty_make_array(ty, array_size),
                                        }));
    }

    cmm_panic("bad ast tree");
}

/// FunDec: ID LP VarList RP | ID LP RP
/// TODO
tret trans_fun_dec(CMM_AST_NODE* node, struct TargFunDec args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_FunDec) cmm_panic("bad ast tree");

    CMM_AST_NODE* id        = node->nodes + 0;
    char*         func_name = id->data.val_ident;

    gen_ir_function(func_name);

    CMM_SEM_TYPE fnty;
    if (node->len == 3) {
        CMM_SEM_TYPE* inner = malloc(sizeof(CMM_SEM_TYPE));
        // ID LP RP
        *inner              = args.return_ty;
        fnty                = cmm_ty_make_func(inner, 1);
    } else if (node->len == 4) {
        CMM_SEM_TYPE* inner    = NULL;
        int           list_len = 0;

        trans_var_list(node->nodes + 2,
                       (struct TargVarList){
                           .list_len  = &list_len,
                           .arg_types = &inner,
                       });

        inner[list_len++] = args.return_ty;

        fnty = cmm_ty_make_func(inner, list_len);
    } else {
        cmm_panic("bad ast tree");
    }

    node->trans.type  = fnty;
    node->trans.ident = func_name;

    // TODO 判断dec有误？
    {
        /// 函数定义在上层作用域
        /// 先修改当前作用域，然后再恢复
        TransScope* current_scope = trans_scope;
        trans_scope               = parent_trans_scope(trans_scope);

        push_trans_def((TransDef){
            .line = id->location.line,
            .kind = TRANS_CTX_VAR,
            .name = func_name,
            .ty   = fnty,
        });

        trans_scope = current_scope;
    }

    RETURN_WITH_TRACE(0);
}

/// VarList: ParamDec COMMA VarList | ParamDec
/// TODO
tret trans_var_list(CMM_AST_NODE* root, struct TargVarList args) {
    CMM_AST_NODE* node = root;
    FUNCTION_TRACE;
    int           len       = 0;
    /// 都256个参数了不至于这都不够吧？？
    CMM_SEM_TYPE* arg_types = malloc(sizeof(CMM_SEM_TYPE) * 256);
    while (true) {
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_VarList) cmm_panic("bad ast tree");

        // TODO
        if (node->len != 1 && node->len != 3) cmm_panic("bad ast tree");

        trans_param_dec(node->nodes + 0,
                        (struct TargParamDec){
                            .ty = arg_types + len,
                        });

        len++;

        if (len >= 255) { printf("panic: too many arguments"); }

        if (node->len == 1) {
            *args.list_len  = len;
            *args.arg_types = arg_types;
            RETURN_WITH_TRACE(0);
        } else {
            node = node->nodes + 2;
        }
    }
}

/// ParamDec: Specifier VarDec
/// TODO
tret trans_param_dec(CMM_AST_NODE* node, struct TargParamDec args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_ParamDec) cmm_panic("bad ast tree");
    if (node->len != 2) cmm_panic("bad ast tree");

    // TODO
    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* vardec    = node->nodes + 1;

    trans_specifier(specifier, (struct TargSpecifier){._void = 0});

    CMM_SEM_TYPE spec_ty = specifier->trans.type;

    TretVarDec var = trans_var_dec(vardec,
                                   (struct TargVarDec){
                                       .ty    = spec_ty,
                                       .where = INSIDE_A_BLOCK,
                                   });

    gen_ir_param(var.bind);

    const TransDef* ctx = get_trans_defination(var.varname);
    *args.ty            = ctx->ty;
    args.ty->bind       = vardec->trans.ident;

    RETURN_WITH_TRACE(0);
}

/// CompSt: LC DefList StmtList RC
/// TODO
tret trans_comp_st(CMM_AST_NODE* node, struct TargCompSt args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_CompSt) cmm_panic("bad ast tree");

    if (node->len != 4) cmm_panic("bad ast tree");

    // LC DefList StmtList RC
    CMM_AST_NODE* deflist  = node->nodes + 1;
    CMM_AST_NODE* stmtlist = node->nodes + 2;

    enter_trans_scope("&");

    // TODO
    trans_def_list(deflist, (struct TargDefList){.where = INSIDE_A_BLOCK});
    trans_stmt_list(stmtlist, (struct TargStmtList){.current_fn_ty = args.current_fn_ty});

    exit_trans_scope();

    RETURN_WITH_TRACE(0);
}

/// StmtList: /* empty */ | Stmt StmtList
/// TODO
tret trans_stmt_list(CMM_AST_NODE* root, struct TargStmtList args) {
    CMM_AST_NODE* node = root;
    FUNCTION_TRACE;
    while (true) {
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_StmtList) cmm_panic("bad ast tree");

        if (node->len == 0) {
            break;
        } else if (node->len == 2) {
            // TODO
            // Stmt StmtList
            trans_stmt(node->nodes + 0,
                       (struct TargStmt){.current_fn_ty = args.current_fn_ty});

            node = node->nodes + 1;
        } else {
            cmm_panic("bad ast tree");
        }
    }

    RETURN_WITH_TRACE(0);
}

// Stmt: Exp SEMI
//     | CompSt
//     | RETURN Exp SEMI
//     | IF LP Exp RP Stmt
//     | IF LP Exp RP Stmt ELSE Stmt
//     | WHILE LP Exp RP Stmt
/// TODO
tret trans_stmt(CMM_AST_NODE* node, struct TargStmt args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Stmt) cmm_panic("bad ast tree");

    CMM_AST_NODE* first = node->nodes + 0;
    switch (first->token) {
        case CMM_TK_Exp: trans_exp(first, (struct TargExp){._void = 0}); break;
        case CMM_TK_CompSt:
            trans_comp_st(first,
                          (struct TargCompSt){.current_fn_ty = args.current_fn_ty});
            break;
        case CMM_TK_RETURN: {
            CMM_AST_NODE* exp = node->nodes + 1;
            trans_exp(exp, (struct TargExp){._void = 0});
            CMM_SEM_TYPE need = args.current_fn_ty.inner[args.current_fn_ty.size - 1];
            CMM_SEM_TYPE got  = exp->trans.type;
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
            trans_exp(exp, (struct TargExp){._void = 0});
            trans_stmt(node->nodes + 4,
                       (struct TargStmt){.current_fn_ty = args.current_fn_ty});
            if (node->len == 7) {
                // IF LP Exp RP Stmt ELSE Stmt
                trans_stmt(node->nodes + 6,
                           (struct TargStmt){.current_fn_ty = args.current_fn_ty});
            }
            CMM_SEM_TYPE exp_ty = exp->trans.type;
            if (exp_ty.kind != CMM_ERROR_TYPE && strcmp(exp_ty.name, "int") != 0) {
                cmm_panic("CMM_SE_OPERAND_TYPE_ERROR");
            }
            break;
        }
        case CMM_TK_WHILE: {
            CMM_AST_NODE* exp = node->nodes + 2;
            trans_exp(exp, (struct TargExp){._void = 0});
            trans_stmt(node->nodes + 4,
                       (struct TargStmt){.current_fn_ty = args.current_fn_ty});
            CMM_SEM_TYPE exp_ty = exp->trans.type;
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

    RETURN_WITH_TRACE(0);
}

/// DefList: /* empty */ | Def DefList
/// TODO
tret trans_def_list(CMM_AST_NODE* root, struct TargDefList args) {
    CMM_AST_NODE* node = root;
    FUNCTION_TRACE;

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

            break;
        } else if (node->len == 2) {
            // TODO
            // Def DefList
            trans_def(
                node->nodes + 0,
                (struct TargDef){
                    .where = args.where, .fill_into = fields, .offset = &fileds_len});

            node = node->nodes + 1;
        } else {
            cmm_panic("bad ast tree");
        }
    }

    RETURN_WITH_TRACE(0);
}

/// Def: Specifier DecList SEMI
/// TODO
tret trans_def(CMM_AST_NODE* node, struct TargDef args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Def) cmm_panic("bad ast tree");

    if (node->len != 3) cmm_panic("bad ast tree");

    // Specifier DecList SEMI
    CMM_AST_NODE* specifier = node->nodes + 0;
    CMM_AST_NODE* declist   = node->nodes + 1;

    trans_specifier(specifier, (struct TargSpecifier){._void = 0});

    CMM_SEM_TYPE spec_ty = specifier->trans.type;

    trans_dec_list(declist,
                   (struct TargDecList){.ty        = spec_ty,
                                        .where     = args.where,
                                        .fill_into = args.fill_into,
                                        .offset    = args.offset});

    RETURN_WITH_TRACE(0);
}

/// DecList: Dec | Dec COMMA DecList
/// TODO
tret trans_dec_list(CMM_AST_NODE* root, struct TargDecList args) {
    CMM_AST_NODE* node = root;
    FUNCTION_TRACE;
    while (true) {
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_DecList) cmm_panic("bad ast tree");
        if (node->len != 1 && node->len != 3) { cmm_panic("bad ast tree"); }

        trans_dec(node->nodes + 0,
                  (struct TargDec){.ty        = args.ty,
                                   .where     = args.where,
                                   .fill_into = args.fill_into,
                                   .offset    = args.offset});

        if (node->len == 1) {
            // Dec
            break;
        } else if (node->len == 3) {
            // Dec COMMA DecList
            node = node->nodes + 2;
        }
    }

    RETURN_WITH_TRACE(0);
}

/// Dec: VarDec | VarDec ASSIGNOP Exp
tret trans_dec(CMM_AST_NODE* node, struct TargDec args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Dec) cmm_panic("bad ast tree");

    if (node->len != 1 && node->len != 3) cmm_panic("bad ast tree");

    CMM_AST_NODE* vardec = node->nodes + 0;

    TretVarDec ret_vardec = trans_var_dec(vardec,
                                          (struct TargVarDec){
                                              .where = args.where,
                                              .ty    = args.ty,
                                          });

    args.fill_into[*args.offset] = ret_vardec.type;
    *args.offset                 = *args.offset + 1;

    if (node->len == 3) {
        if (args.where == INSIDE_A_STRUCT) { cmm_panic("CMM_SE_BAD_STRUCT_DOMAIN"); }
        CMM_AST_NODE* exp = node->nodes + 2;
        trans_exp(exp, (struct TargExp){._void = 0});
        // 赋值语句的类型检查
        if (!cmm_ty_fitable(exp->trans.type, args.ty)) {
            cmm_panic("CMM_SE_ASSIGN_TYPE_ERROR");
        }
    }

    RETURN_WITH_TRACE(0);
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
tret trans_exp(CMM_AST_NODE* node, struct TargExp args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Exp) cmm_panic("bad ast tree");

    node->trans.type       = cmm_ty_make_error();
    node->trans.value_kind = RVALUE;

    CMM_AST_NODE* a = node->nodes + 0;

    if (a->token == CMM_TK_Exp) {
        trans_exp(a, args);

        CMM_AST_NODE* op     = node->nodes + 1;
        CMM_AST_NODE* b      = node->nodes + 2;
        CMM_SEM_TYPE  type_a = a->trans.type;

        /// struct.field
        if (op->token == CMM_TK_DOT) {
            node->trans.value_kind = LVALUE;
            trans_exp(b, args);
            CMM_SEM_TYPE type_b = b->trans.type;

            switch (op->token) {
                case CMM_TK_ASSIGNOP: node->trans.type = type_a; break;
                case CMM_TK_AND:
                case CMM_TK_OR: node->trans.type = type_b; break;
                case CMM_TK_RELOP: node->trans.type = cmm_ty_make_primitive("int"); break;
                case CMM_TK_PLUS:
                case CMM_TK_MINUS:
                case CMM_TK_STAR:
                case CMM_TK_DIV: node->trans.type = type_b; break;
                /// array[idx]
                case CMM_TK_LB:
                    node->trans.value_kind = LVALUE;
                    node->trans.type       = *type_a.inner;
                    break;
                default: cmm_panic("bad ast tree");
            }
        }
    } else if (a->token == CMM_TK_ID) {
        const char*     a_name = a->data.val_ident;
        const TransDef* a_def  = get_trans_defination(a_name);

        if (a_def == NULL) {
            if (node->len == 1) {
                cmm_panic("variable %s is not fount", a_name);
            } else {
                cmm_panic("function %s is not fount", a_name);
            }
        } else {
            node->trans.type = a_def->ty;

#ifdef CMM_DEBUG_LAB3TRACE
            cmm_debug(COLOR_MAGENTA, "// %s : %s\n", a_name, a_def->ty.name);
#endif
        }

        if (node->len == 1) {
            node->trans.value_kind = LVALUE;
            RETURN_WITH_TRACE(0);
        }

        if (a_def->ty.kind != CMM_FUNCTION_TYPE) {
            cmm_panic("CMM_SE_BAD_FUNCTION_CALL");
        }

        node->trans.type = a_def->ty.inner[a_def->ty.size - 1];

        if (node->len == 3) {
            if (a_def->ty.size != 1) { cmm_panic("CMM_SE_ARGS_NOT_MATCH"); }
        } else if (node->len == 4) {
            CMM_AST_NODE* arg_node = node->nodes + 2;
            trans_args(arg_node, (struct TargArgs){.calling = a_def->ty});
        }
    } else if (a->token == CMM_TK_MINUS) {
        CMM_AST_NODE* b = node->nodes + 1;
        trans_exp(b, args);
        CMM_SEM_TYPE type_b = b->trans.type;
        node->trans.type    = type_b;
    } else if (a->token == CMM_TK_NOT) {
        CMM_AST_NODE* b = node->nodes + 1;
        trans_exp(b, args);
        CMM_SEM_TYPE type_b = b->trans.type;
        node->trans.type    = type_b;
    } else if (a->token == CMM_TK_LP) {
        // 括号
        CMM_AST_NODE* b = node->nodes + 1;
        trans_exp(b, args);
        CMM_SEM_TYPE type_b    = b->trans.type;
        node->trans.type       = type_b;
        node->trans.value_kind = b->trans.value_kind;
    } else if (a->token == CMM_TK_INT) {
        node->trans.type = cmm_ty_make_primitive("int");
    } else if (a->token == CMM_TK_FLOAT) {
        node->trans.type = cmm_ty_make_primitive("float");
    } else {
        cmm_panic("bad ast tree");
    }

    RETURN_WITH_TRACE(0);
}


// Args: Exp COMMA Args
//     | Exp
tret trans_args(CMM_AST_NODE* root, struct TargArgs args) {
    CMM_AST_NODE* node = root;
    FUNCTION_TRACE;

    // args.calling.size >= 2
    int return_type_idx = args.calling.size - 1;
    int tail_idx        = args.calling.size - 2;

    for (int i = 0;; i++) {
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_Args) cmm_panic("bad ast tree");
        if (node->len != 1 && node->len != 3) cmm_panic("bad ast tree");
        if (i == return_type_idx) { cmm_panic("CMM_SE_ARGS_NOT_MATCH"); }

        CMM_AST_NODE* param = node->nodes + 0;

        trans_exp(param, (struct TargExp){._void = 0});

        if (!cmm_ty_fitable(args.calling.inner[i], param->trans.type)) {
            cmm_panic("CMM_SE_ARGS_NOT_MATCH");
        }

        if (node->len == 1) {
            if (i != tail_idx) { cmm_panic("CMM_SE_ARGS_NOT_MATCH"); }
            break;
        } else if (node->len == 3) {
            node = node->nodes + 2;
        }
    }

    root->trans.type = args.calling.inner[return_type_idx];

    RETURN_WITH_TRACE(0);
}

#pragma endregion