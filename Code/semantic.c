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

enum CMM_SEMANTIC analyze_program(CMM_AST_NODE* node) {
    if (node == NULL) { return CMM_SE_OK; }
    if (node->kind != CMM_TK_Program) { return CMM_SE_BAD_AST_TREE; }
}

enum CMM_SEMANTIC analyze_ext_def_list(CMM_AST_NODE* node) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_ExtDefList) { return CMM_SE_BAD_AST_TREE; }

    if (node->len == 0) { return CMM_SE_OK; }

    if (node->len == 2) {
        // TODO
        analyze_ext_def(node->nodes + 0);
        ANALYZE_EXPECT_OK(analyze_ext_def_list(node->nodes + 1));
        return CMM_SE_OK;
    }

    return CMM_SE_BAD_AST_TREE;
}

enum CMM_SEMANTIC analyze_ext_def(CMM_AST_NODE* node) {
    if (node == NULL) { return CMM_SE_BAD_AST_TREE; }
    if (node->kind != CMM_TK_ExtDef) { return CMM_SE_BAD_AST_TREE; }

    analyze_specifier(node->nodes);
    // TODO
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
enum CMM_SEMANTIC analyze_ext_dec_list(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_fun_dec(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_comp_st(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_var_dec(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_struct_specifier(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_opt_tag(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_def_list(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_tag(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_var_list(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_param_dec(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_stmt_list(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_stmt(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_exp(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_def(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_dec_list(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_dec(CMM_AST_NODE* node);




/// TODO
enum CMM_SEMANTIC analyze_args(CMM_AST_NODE* node);
