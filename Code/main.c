#include "predefines.h"
#include "globals.h"
#include "semantic.h"
#include "syndef.h"
#include "syntax.tab.h"
#include <stdio.h>

void cmm_print_node(CMM_AST_NODE* val, int indent) {
    if (val->kind == CMM_AST_NODE_TREE && val->len == 0) { return; }
    for (int i = 0; i < indent; i++) { printf(" "); }

    switch (val->kind) {
        case CMM_AST_NODE_TOKEN: {
            printf("%s\n", cmm_token_tostring(val->token));
            break;
        }
        case CMM_AST_NODE_INT: {
            printf("INT: %d\n", val->data.val_int);
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
            printf("%s (%d)\n", cmm_token_tostring(val->token), val->location.line);
            for (int i = 0; i < val->len; i++) {
                cmm_print_node(val->nodes + i, indent + 2);
            }
            break;
        }
    }
}

extern void yyrestart(FILE*);

int main(int argc, char** argv) {
    if (argc <= 1) return 1;

    FILE* f = fopen(argv[1], "r");

#ifdef CMM_DEBUG_FLAG
    printf("\n======== BEGIN ==========\n\n");
#endif

    yyrestart(f);
    yyparse();
    if (cmm_lexical_error) { return 0; }
    if (cmm_syntax_error) { return 0; }

#ifdef CMM_DEBUG_FLAG
    printf("\n\n======================\n\n");
    cmm_log_node(&cmm_parsed_root);
    printf("\n\n======================\n\n");
#endif

#ifdef BYYL_IS_LAB1
    cmm_print_node(&cmm_parsed_root, 0);
#endif

    int sem_error_cnt = cmm_semantic_analyze(&cmm_parsed_root);

    if (sem_error_cnt > 0) {
        CMM_SEMANTIC_ERROR* errors = cmm_get_semantic_errors();
        for (int i = 1; i <= sem_error_cnt; i++) {
            printf("Error type %d at Line %d: %s\n",
                   errors[i].type,
                   errors[i].line,
                   cmm_semantic_error_to_string(errors[i].type));
        }
    }
}