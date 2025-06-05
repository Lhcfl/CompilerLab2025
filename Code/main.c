#include "codegen.h"
#include "ir.h"
#include "llib.h"
#include "predefines.h"
#include "globals.h"
#include "semantic.h"
#include "translate.h"
#include "syndef.h"
#include "syntax.tab.h"
#include "stdio.h"

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
            printf("%s (%d)\n",
                   cmm_token_tostring(val->token),
                   val->location.line);
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

#ifdef CMM_DEBUG_LAB1
    printf("\n======== BEGIN ==========\n\n");
#endif

    yyrestart(f);
    yyparse();
    if (cmm_lexical_error) { return 0; }
    if (cmm_syntax_error) { return 0; }

#ifdef CMM_DEBUG_LAB1
    printf("\n\n======================\n\n");
    cmm_log_node(&cmm_parsed_root);
    printf("\n\n======================\n\n");
#endif

#ifdef CMM_DEBUG_LAB2
    printf("\n\n======================\n\n");
    cmm_log_node(&cmm_parsed_root);
    printf("\n\n======================\n\n");
#endif

#ifdef BYYL_IS_LAB1
    cmm_print_node(&cmm_parsed_root, 0);
#endif

#ifdef BYYL_IS_LAB2
    int sem_error_cnt = cmm_semantic_analyze(&cmm_parsed_root);

    if (sem_error_cnt > 0) {
        CMM_SEMANTIC_ERROR* errors = cmm_get_semantic_errors();
        for (int i = 0; i < sem_error_cnt; i++) {
            printf("Error type %d at Line %d: %s\n",
                   errors[i].type,
                   errors[i].line,
                   cmm_semantic_error_to_string(errors[i].type));
        }
    }
#endif

    initialize_ir();
    cmm_trans_code(&cmm_parsed_root);

    FILE* ir = fopen(argv[2], "w");

#ifdef BYYL_IS_LAB3
    for (size_t i = 0; i < get_ir_output()->size; i++) {
        fprintf(ir, "%s\n", ptr[i]->str);
    }
#endif

    cmm_gen_code();

    LString* asm_output = get_asm_output()->data;
    for (size_t i = 0; i < get_asm_output()->size; i++) {
        fprintf(ir, "%s\n", asm_output[i]->str);
    }

    IR_CODE* ptr = get_ir_output()->data;
    for (size_t i = 0; i < get_ir_output()->size; i++) {
        LString str = ir_code_to_string(ptr[i]);
        fprintf(ir, "%s\n", str->str);
        LStringFree(str);
    }

    fclose(ir);
    free_asm_output();
    free_all_ir();
    cmm_free_ast(&cmm_parsed_root);
}