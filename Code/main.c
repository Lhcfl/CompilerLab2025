#include "predefines.h"
#include "globals.h"
#include "globals.cpp"
#include "syntax.tab.h"
#include <stdio.h>

void cmm_print_node(CMM_AST_NODE* val, int indent) {
    if (val->kind == CMM_AST_NODE_TREE && val->len == 0) { return; }
    for (int i = 0; i <= indent; i++) { printf(" "); }

    switch (val->kind) {
        case CMM_AST_NODE_TOKEN: {
            printf("%s\n", val->data.val_token);
            break;
        }
        case CMM_AST_NODE_INT: {
            printf("INT: %d \n", val->data.val_int);
            break;
        }
        case CMM_AST_NODE_FLOAT: {
            printf("FLOAT: %f\n", val->data.val_float);
            break;
        }
        case CMM_AST_NODE_TYPE: {
            printf("TYPE: %s\n", val->data.val_type);
            break;
        }
        case CMM_AST_NODE_IDENT: {
            printf("ID: %s\n", val->data.val_ident);
            break;
        }
        case CMM_AST_NODE_TREE: {
            printf("%s (%d)\n", val->data.val_tree_name, val->location.line);
            for (int i = 0; i < val->len; i++) {
                cmm_print_node(val->nodes + i, indent + 2);
            }
            break;
        }
    }
}

int main() {
    printf("\n=====================\n\n");
    yyparse();
    printf("\n\n======================\n\n");
    cmm_log_node(&cmm_parsed_root);
    printf("\n\n======================\n\n");
    cmm_print_node(&cmm_parsed_root, 0);
    printf("done");
}