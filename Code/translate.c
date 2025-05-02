#include "translate.h"
#include "hashmap.h"
#include "ir.h"
#include "predefines.h"
#include "syndef.h"
#include <stdbool.h>
#include <stddef.h>
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
#    define FUNCTION_TRACE                                                    \
        {                                                                     \
            __trans_trace_spaces++;                                           \
            for (int i = 0; i < __trans_trace_spaces; i++) printf(" ");       \
            printf(COLOR_BLUE "%s : %s:%d : ", __func__, __FILE__, __LINE__); \
            cmm_debug_show_node_info(node, 2);                                \
            printf("\n\033[0m");                                              \
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
    enum TransDefKind {
        TRANS_DEF_TYPE,
        TRANS_DEF_VAR,
        TRANS_DEF_VAR_ADDR,
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
    int _void;
};

typedef struct TretOptTag {
    char* name;
} TretOptTag;

typedef struct TretVarDec {
    char*        varname;
    CMM_IR_VAR   bind;
    CMM_SEM_TYPE type;
} TretVarDec;

typedef struct TretExp {
    CMM_IR_VAR          bind;
    CMM_SEM_TYPE        type;
    enum CMM_VALUE_KIND vkind;
    bool                is_ident;
    bool                is_struct_addr;
    /// lvalue 的 addr
    CMM_IR_VAR          addr;
} TretExp;

const TransDef* get_trans_defination(const char* name);

tret       trans_program(CMM_AST_NODE* node, struct TargProgram args);
tret       trans_ext_def_list(CMM_AST_NODE* node, struct TargExtDefList args);
tret       trans_ext_def(CMM_AST_NODE* node, struct TargExtDef args);
tret       trans_specifier(CMM_AST_NODE* node, struct TargSpecifier args);
tret       trans_ext_dec_list(CMM_AST_NODE* node, struct TargExtDecList args);
tret       trans_fun_dec(CMM_AST_NODE* node, struct TargFunDec args);
tret       trans_comp_st(CMM_AST_NODE* node, struct TargCompSt args);
TretVarDec trans_var_dec(CMM_AST_NODE* node, struct TargVarDec args);
tret       trans_struct_specifier(CMM_AST_NODE*              node,
                                  struct TargStructSpecifier args);
TretOptTag trans_opt_tag(CMM_AST_NODE* node, struct TargOptTag args);
tret       trans_def_list(CMM_AST_NODE* node, struct TargDefList args);
tret       trans_tag(CMM_AST_NODE* node, struct TargTag args);
tret       trans_var_list(CMM_AST_NODE* node, struct TargVarList args);
tret       trans_param_dec(CMM_AST_NODE* node, struct TargParamDec args);
tret       trans_stmt_list(CMM_AST_NODE* node, struct TargStmtList args);
tret       trans_stmt(CMM_AST_NODE* node, struct TargStmt args);
TretExp    trans_exp(CMM_AST_NODE* node, struct TargExp args);
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

int trans_DEF_compare(const void* a, const void* b, void* _udata) {
    const TransDef* ua = a;
    const TransDef* ub = b;
    (void)(_udata);
    return strcmp(ua->name, ub->name);
}

uint64_t trans_DEF_hash(const void* item, uint64_t seed0, uint64_t seed1) {
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
        TransDef* deleted = (TransDef*)hashmap_delete(
            trans_context, &(TransDef){.name = ptr->name});

        // if (deleted->def == TRANS_DEF_DECLARE) {
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
TransScope* enter_trans_scope(const char* name) {
    return __enter_trans_scope(name);
}
void exit_trans_scope() { __exit_trans_scope(); }
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

char* _trans_DEF_finder(const char* name, TransScope* scope) {
    if (scope == NULL) {
        return cmm_clone_string(name);
    } else {
        char* tmp = _trans_DEF_finder(scope->name, scope->back);
        if (name == NULL) { return tmp; }
        char* ret = cmm_concat_string(3, tmp, "::", name);
        free(tmp);
        return ret;
    }
}

/// 在当前作用域下产生一个新的定义或声明
void push_trans_def(TransDef arg) {
    char* varname = _trans_DEF_finder(arg.name, trans_scope);

    // 检查是否存在这个def
    TransDef* existed =
        (TransDef*)hashmap_get(trans_context, &(TransDef){.name = varname});

    if (existed != NULL)
        cmm_panic("defined again %s : %s", varname, arg.ty.name);

    /// 变量的名字不能和类型一样
    const TransDef* existed_rec = get_trans_defination(arg.name);
    if (existed_rec != NULL && existed_rec->kind == TRANS_DEF_TYPE &&
        arg.kind == TRANS_DEF_VAR)
        cmm_panic("var name is same as type name %s : %s",
                  varname,
                  existed_rec->ty.name);

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
        char* varname = _trans_DEF_finder(name, scope);
        // #ifdef CMM_DEBUG_LAB3TRACE
        //         printf("[search] finding %s\n", varname);
        // #endif
        ret = hashmap_get(trans_context, &(TransDef){.name = varname});
        free(varname);
        scope = scope->back;
    }

    // #ifdef CMM_DEBUG_LAB3TRACE
    //     if (ret == NULL) { cmm_debug(COLOR_CYAN, "[search] %s not found\n",
    //     name); }
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
                                trans_DEF_hash,
                                trans_DEF_compare,
                                free_trans_ctx,
                                NULL);

    enter_trans_scope("root");
    root_trans_scope = trans_scope;

    // read: () -> int
    push_trans_def((TransDef){
        .kind = TRANS_DEF_VAR,
        .name = cmm_clone_string("read"),
        .ty   = cmm_create_function_type(1, "int"),
        .line = 0,
    });

    // write: int -> int
    push_trans_def((TransDef){
        .kind = TRANS_DEF_VAR,
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
            trans_comp_st(
                node->nodes + 2,
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
            .kind = TRANS_DEF_TYPE,
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
        CMM_IR_VAR ir_var = ir_new_var(id);

        push_trans_def((TransDef){
            .kind   = TRANS_DEF_VAR,
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

        if (args.ty.kind == CMM_ARRAY_TYPE) {
            cmm_panic("lab3 does not support array of array");
        }

        RETURN_WITH_TRACE(
            trans_var_dec(vardec,
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
            .kind = TRANS_DEF_VAR,
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

    if (var.type.kind == CMM_ARRAY_TYPE) {
        cmm_panic("lab3 does not support array as function argument");
    }

    gen_ir_param(var.bind);

    TransDef* ctx = (TransDef*)get_trans_defination(var.varname);
    *args.ty      = ctx->ty;
    args.ty->bind = vardec->trans.ident;

    if (ctx->ty.kind != CMM_PRIMITIVE_TYPE) { ctx->kind = TRANS_DEF_VAR_ADDR; }

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
    trans_stmt_list(stmtlist,
                    (struct TargStmtList){.current_fn_ty = args.current_fn_ty});

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
            trans_comp_st(
                first,
                (struct TargCompSt){.current_fn_ty = args.current_fn_ty});
            break;
        case CMM_TK_RETURN: {
            CMM_AST_NODE* exp = node->nodes + 1;
            TretExp       ret = trans_exp(exp, (struct TargExp){._void = 0});
            gen_ir_return(ret.bind);
            break;
        }
        case CMM_TK_IF: {
            CMM_IR_LABEL  lbl_then = ir_new_label("then");
            CMM_IR_LABEL  lbl_else = ir_new_label("else");
            CMM_IR_LABEL  lbl_nxt  = ir_new_label("nxt");
            CMM_AST_NODE* exp      = node->nodes + 2;
            TretExp       check = trans_exp(exp, (struct TargExp){._void = 0});

            gen_ir_if_goto(check.bind, ir_new_immediate_int(0), "==", lbl_else);

            gen_ir_label_start(lbl_then);

            trans_stmt(node->nodes + 4,
                       (struct TargStmt){.current_fn_ty = args.current_fn_ty});

            gen_ir_goto(lbl_nxt);

            gen_ir_label_start(lbl_else);

            if (node->len == 7) {
                // IF LP Exp RP Stmt ELSE Stmt
                trans_stmt(
                    node->nodes + 6,
                    (struct TargStmt){.current_fn_ty = args.current_fn_ty});
            }

            gen_ir_label_start(lbl_nxt);
            break;
        }
        case CMM_TK_WHILE: {
            CMM_IR_LABEL  lbl_body = ir_new_label("while");
            CMM_IR_LABEL  lbl_nxt  = ir_new_label("nxt");
            CMM_AST_NODE* exp      = node->nodes + 2;

            gen_ir_label_start(lbl_body);

            TretExp check = trans_exp(exp, (struct TargExp){._void = 0});

            gen_ir_if_goto(check.bind, ir_new_immediate_int(0), "==", lbl_nxt);

            trans_stmt(node->nodes + 4,
                       (struct TargStmt){.current_fn_ty = args.current_fn_ty});

            gen_ir_goto(lbl_body);
            gen_ir_label_start(lbl_nxt);
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
            trans_def(node->nodes + 0,
                      (struct TargDef){.where     = args.where,
                                       .fill_into = fields,
                                       .offset    = &fileds_len});

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

    if (ret_vardec.type.kind == CMM_ARRAY_TYPE ||
        ret_vardec.type.kind == CMM_PROD_TYPE) {
        gen_ir_alloc(ret_vardec.bind, ret_vardec.type.bytes4);
    }

    if (node->len == 3) {
        if (args.where == INSIDE_A_STRUCT) {
            cmm_panic("CMM_SE_BAD_STRUCT_DOMAIN");
        }
        CMM_AST_NODE* exp = node->nodes + 2;
        TretExp       ret = trans_exp(exp, (struct TargExp){._void = 0});

        gen_ir_assign(ret_vardec.bind, ret.bind);
    }

    RETURN_WITH_TRACE(0);
}

void translate_function_call(CMM_IR_VAR    ret,
                             const char*   func_name,
                             CMM_AST_NODE* args) {
    if (strcmp(func_name, "read") == 0) {
        gen_ir_read(ret);
        return;
    } else if (strcmp(func_name, "write") == 0) {
        CMM_AST_NODE* exp0 = args->nodes + 0;
        TretExp       exp  = trans_exp(exp0, (struct TargExp){._void = 0});
        gen_ir_write(exp.bind);
        gen_ir_assign(ret, ir_new_immediate_int(0));
        return;
    }

    // TODO
    // 1. 生成函数调用的 IR
    // 2. 检查参数个数和类型
    // 3. 生成参数的 IR

    if (args != NULL) { trans_args(args, (struct TargArgs){._void = 0}); }

    gen_ir_call(ret, func_name);
}

CMM_IR_VAR translate_relop(CMM_IR_VAR a, const char* relop, CMM_IR_VAR b) {
    CMM_IR_VAR   ret     = ir_new_tmpvar();
    CMM_IR_LABEL is_true = ir_new_label("true");
    CMM_IR_LABEL nxt     = ir_new_label("nxt");

    gen_ir_if_goto(a, b, relop, is_true);
    // false
    gen_ir_assign(ret, ir_new_immediate_int(0));
    gen_ir_goto(nxt);
    // true
    gen_ir_label_start(is_true);
    gen_ir_assign(ret, ir_new_immediate_int(1));
    // nxt
    gen_ir_label_start(nxt);

    return ret;
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
TretExp trans_exp(CMM_AST_NODE* node, struct TargExp args) {
    FUNCTION_TRACE;
    if (node == NULL) cmm_panic("bad ast tree");
    if (node->token != CMM_TK_Exp) cmm_panic("bad ast tree");

    CMM_AST_NODE* node_a = node->nodes + 0;

    TretExp ret;
    ret.is_ident = false;
    ret.vkind    = RVALUE;

    if (node_a->token == CMM_TK_Exp) {
        TretExp a = trans_exp(node_a, args);

        CMM_AST_NODE* op     = node->nodes + 1;
        CMM_AST_NODE* node_b = node->nodes + 2;

        /// struct.field
        if (op->token == CMM_TK_DOT) {
            ret.vkind    = LVALUE;
            char* b_name = node_b->data.val_ident;
            ret.type     = *cmm_ty_field_of_struct(a.type, b_name);
            ret.bind     = ir_new_tmpvar();
            ret.is_ident = false;

            CMM_IR_VAR a_addr = ir_new_tmpvar();
            ret.addr          = ir_new_tmpvar();

#ifdef CMM_DEBUG_LAB3TRACE
            cmm_debug(COLOR_MAGENTA,
                      "// struct addr? %s\n",
                      a.is_struct_addr ? "true" : "false");
#endif
            if (a.is_struct_addr) {
                gen_ir_assign(a_addr, a.bind);
            } else {
                gen_ir_get_addr(a_addr, a.bind);
            }

            CMM_IR_VAR offset = ir_new_immediate_int(
                cmm_offset_of_struct_field(a.type, b_name) * 4);

            gen_ir_add(ret.addr, a_addr, offset);
            gen_ir_dereference(ret.bind, ret.addr);
            RETURN_WITH_TRACE(ret);
        }

        switch (op->token) {
            case CMM_TK_ASSIGNOP: {
                TretExp b = trans_exp(node_b, args);
                ret.vkind = LVALUE;
                ret.type  = b.type;
                ret.bind  = b.bind;

#ifdef CMM_DEBUG_LAB3TRACE
                cmm_debug(COLOR_MAGENTA,
                          "// assign %s\n",
                          a.is_ident ? "ident" : "arr");
#endif

                if (!a.is_ident) {
                    gen_ir_put_into_addr(a.addr, b.bind);
                } else {
                    gen_ir_assign(a.bind, b.bind);
                }

                RETURN_WITH_TRACE(ret);
            }
            case CMM_TK_AND: {
                ret.type = a.type;
                ret.bind = ir_new_tmpvar();

                CMM_IR_LABEL lbl_ret_false = ir_new_label("is_false");
                CMM_IR_LABEL lbl_nxt       = ir_new_label("nxt");

                gen_ir_if_goto(
                    a.bind, ir_new_immediate_int(0), "==", lbl_ret_false);

                TretExp b = trans_exp(node_b, args);

                gen_ir_if_goto(
                    b.bind, ir_new_immediate_int(0), "==", lbl_ret_false);

                gen_ir_assign(ret.bind, ir_new_immediate_int(1));
                gen_ir_goto(lbl_nxt);

                gen_ir_label_start(lbl_ret_false);
                gen_ir_assign(ret.bind, ir_new_immediate_int(0));

                gen_ir_label_start(lbl_nxt);

                RETURN_WITH_TRACE(ret);
            }
            case CMM_TK_OR: {
                ret.type = a.type;
                ret.bind = ir_new_tmpvar();

                CMM_IR_LABEL lbl_ret_true = ir_new_label("is_true");
                CMM_IR_LABEL lbl_nxt      = ir_new_label("nxt");

                gen_ir_if_goto(
                    a.bind, ir_new_immediate_int(0), "!=", lbl_ret_true);

                TretExp b = trans_exp(node_b, args);

                gen_ir_if_goto(
                    b.bind, ir_new_immediate_int(0), "!=", lbl_ret_true);

                gen_ir_assign(ret.bind, ir_new_immediate_int(0));
                gen_ir_goto(lbl_nxt);

                gen_ir_label_start(lbl_ret_true);
                gen_ir_assign(ret.bind, ir_new_immediate_int(1));

                gen_ir_label_start(lbl_nxt);
                RETURN_WITH_TRACE(ret);
            }
            case CMM_TK_RELOP: {
                TretExp b = trans_exp(node_b, args);
                ret.type  = cmm_ty_make_primitive("int");
                ret.bind  = translate_relop(a.bind, op->data.val_relop, b.bind);
                RETURN_WITH_TRACE(ret);
            }
            case CMM_TK_PLUS: {
                TretExp b = trans_exp(node_b, args);
                ret.type  = b.type;
                ret.bind  = ir_new_tmpvar();
                gen_ir_add(ret.bind, a.bind, b.bind);
                RETURN_WITH_TRACE(ret);
            }
            case CMM_TK_MINUS: {
                TretExp b = trans_exp(node_b, args);
                ret.type  = b.type;
                ret.bind  = ir_new_tmpvar();
                gen_ir_sub(ret.bind, a.bind, b.bind);
                RETURN_WITH_TRACE(ret);
            }
            case CMM_TK_STAR: {
                TretExp b = trans_exp(node_b, args);
                ret.type  = b.type;
                ret.bind  = ir_new_tmpvar();
                gen_ir_mul(ret.bind, a.bind, b.bind);
                RETURN_WITH_TRACE(ret);
            }
            case CMM_TK_DIV: {
                TretExp b = trans_exp(node_b, args);
                ret.type  = b.type;
                ret.bind  = ir_new_tmpvar();
                gen_ir_div(ret.bind, a.bind, b.bind);
                RETURN_WITH_TRACE(ret);
            }
            /// array[idx]
            case CMM_TK_LB: {
                TretExp b = trans_exp(node_b, args);
                if (!a.is_ident) {
                    cmm_panic("Cannot translate: Code contains variables of "
                              "multi-dimensional array type or parameters of "
                              "array type. ");
                }
                ret.vkind    = LVALUE;
                ret.type     = *a.type.inner;
                ret.bind     = ir_new_tmpvar();
                ret.is_ident = false;

                CMM_IR_VAR arr_addr = ir_new_tmpvar();
                gen_ir_get_addr(arr_addr, a.bind);
                ret.addr          = ir_new_tmpvar();
                CMM_IR_VAR offset = ir_new_tmpvar();

                gen_ir_mul(offset, b.bind, ir_new_immediate_int(4));

                gen_ir_add(ret.addr, arr_addr, offset);
                gen_ir_dereference(ret.bind, ret.addr);
                RETURN_WITH_TRACE(ret);
            }
            default: cmm_panic("bad ast tree");
        }
    } else if (node_a->token == CMM_TK_ID) {
        const char*     a_name = node_a->data.val_ident;
        const TransDef* a_def  = get_trans_defination(a_name);

        if (a_def == NULL) {
            if (node->len == 1) {
                cmm_panic("variable %s is not fount", a_name);
            } else {
                cmm_panic("function %s is not fount", a_name);
            }
        }

#ifdef CMM_DEBUG_LAB3TRACE
        cmm_debug(COLOR_MAGENTA, "// %s : %s\n", a_name, a_def->ty.name);
#endif

        if (node->len == 1) {
            ret.vkind          = LVALUE;
            ret.is_ident       = true;
            ret.type           = a_def->ty;
            ret.bind           = a_def->ir_var;
            ret.is_struct_addr = a_def->kind == TRANS_DEF_VAR_ADDR;
            RETURN_WITH_TRACE(ret);
        }

        if (a_def->ty.kind != CMM_FUNCTION_TYPE) {
            cmm_panic("CMM_SE_BAD_FUNCTION_CALL");
        }

        ret.type = a_def->ty.inner[a_def->ty.size - 1];
        ret.bind = ir_new_tmpvar();

        translate_function_call(
            ret.bind, a_name, node->len == 3 ? NULL : node->nodes + 2);

        RETURN_WITH_TRACE(ret);

    } else if (node_a->token == CMM_TK_MINUS) {
        ret.bind  = ir_new_tmpvar();
        TretExp b = trans_exp(node->nodes + 1, args);
        ret.type  = b.type;
        gen_ir_sub(ret.bind, ir_new_immediate_int(0), b.bind);

        RETURN_WITH_TRACE(ret);

    } else if (node_a->token == CMM_TK_NOT) {
        ret.type  = cmm_ty_make_primitive("int");
        TretExp b = trans_exp(node->nodes + 1, args);
        ret.bind  = translate_relop(b.bind, "==", ir_new_immediate_int(0));

        RETURN_WITH_TRACE(ret);

    } else if (node_a->token == CMM_TK_LP) {

        RETURN_WITH_TRACE(trans_exp(node->nodes + 1, args));

    } else if (node_a->token == CMM_TK_INT) {
        ret.type = cmm_ty_make_primitive("int");
        ret.bind = ir_new_immediate_int(node_a->data.val_int);

        RETURN_WITH_TRACE(ret);

    } else if (node_a->token == CMM_TK_FLOAT) {
        ret.type = cmm_ty_make_primitive("float");
        ret.bind = ir_new_immediate_int(node_a->data.val_float);

        RETURN_WITH_TRACE(ret);
    }

    cmm_panic("bad ast tree");
}


// Args: Exp COMMA Args
//     | Exp
tret trans_args(CMM_AST_NODE* root, struct TargArgs _) {
    (void)_;
    CMM_AST_NODE* node = root;
    FUNCTION_TRACE;

    struct list_of_TretExp {
        TretExp                 data;
        struct list_of_TretExp* next;
    };

    struct list_of_TretExp* head = NULL;

    while (true) {
        if (node == NULL) cmm_panic("bad ast tree");
        if (node->token != CMM_TK_Args) cmm_panic("bad ast tree");
        if (node->len != 1 && node->len != 3) cmm_panic("bad ast tree");

        CMM_AST_NODE* param = node->nodes + 0;

        struct list_of_TretExp* n = malloc(sizeof(struct list_of_TretExp));
        n->data = trans_exp(param, (struct TargExp){._void = 0});
        n->next = head;
        head    = n;

        if (node->len == 1) {
            break;
        } else if (node->len == 3) {
            node = node->nodes + 2;
        }
    }

    // 逆序 push args
    while (head != NULL) {
        CMM_IR_VAR arg = head->data.bind;
        if (head->data.type.kind != CMM_PRIMITIVE_TYPE) {
            gen_ir_arg_addr(arg);
        } else {
            gen_ir_arg(arg);
        }
        struct list_of_TretExp* tmp = head->next;
        free(head);
        head = tmp;
    }

    RETURN_WITH_TRACE(0);
}

#pragma endregion