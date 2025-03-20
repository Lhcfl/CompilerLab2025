#include "predefines.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern int yylex(void);
extern int fileno(FILE*);

void yyerror(char* msg) { fprintf(stderr, "error: %s\n", msg); }

char* cmm_clone_string(const char* str) {
    size_t len   = strlen(str);
    char*  clone = (char*)malloc((len + 1) * sizeof(char));
    if (clone == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    strcpy(clone, str);
    return clone;
}

#define YYSTYPE CMM_AST_NODE

extern YYSTYPE yylval;
extern char*   yytext;

void cmm_log_node(CMM_AST_NODE* val) {
#ifndef CMM_DEBUG_FLAG
    return;
#endif
    switch (val->kind) {
        case CMM_AST_NODE_TOKEN: {
            printf("{ \"kind\": \"token\", \"val\": \"%d\", \"text\": \"%s\" }",
                   val->data.val_token,
                   val->text);
            break;
        }
        case CMM_AST_NODE_INT: {
            printf("{ \"kind\": \"int\", \"val\": %d }", val->data.val_int);
            break;
        }
        case CMM_AST_NODE_FLOAT: {
            printf("{ \"kind\": \"float\", \"val\": %f }", val->data.val_float);
            break;
        }
        case CMM_AST_NODE_TYPE: {
            printf("{ \"kind\": \"type\", \"val\": \"%s\" }", val->data.val_type);
            break;
        }
        case CMM_AST_NODE_IDENT: {
            printf("{ \"kind\": \"ident\", \"val\": \"%s\" }", val->data.val_ident);
            break;
        }
        case CMM_AST_NODE_TREE: {
            printf("{ \"kind\": \"tree\", \"val\": \"%s\", \"child\": [ ",
                   val->data.val_tree_name);
            for (int i = 0; i < val->len; i++) {
                if (i != 0) printf(", ");
                cmm_log_node(val->nodes + i);
            }
            printf(" ] }");
            break;
        }
    }
}

void cmm_send_yylval_token(int yysymbol_kind) {
    yylval.kind           = CMM_AST_NODE_TOKEN;
    yylval.data.val_token = yysymbol_kind;
    yylval.nodes          = NULL;
    yylval.len            = 0;
    yylval.text           = cmm_clone_string(yytext);

    cmm_log_node(&yylval);
}

void cmm_send_yylval_int(int val) {
    yylval.kind         = CMM_AST_NODE_INT;
    yylval.data.val_int = val;
    yylval.nodes        = NULL;
    yylval.len          = 0;
    yylval.text         = cmm_clone_string(yytext);

    cmm_log_node(&yylval);
}

void cmm_send_yylval_float(float val) {
    yylval.kind           = CMM_AST_NODE_INT;
    yylval.data.val_float = val;
    yylval.nodes          = NULL;
    yylval.len            = 0;
    yylval.text           = cmm_clone_string(yytext);

    cmm_log_node(&yylval);
}

void cmm_send_yylval_type(char* val) {
    yylval.kind          = CMM_AST_NODE_TYPE;
    yylval.data.val_type = cmm_clone_string(val);
    yylval.nodes         = NULL;
    yylval.len           = 0;
    yylval.text          = cmm_clone_string(yytext);

    cmm_log_node(&yylval);
}

void cmm_send_yylval_ident(char* val) {
    yylval.kind           = CMM_AST_NODE_IDENT;
    yylval.data.val_ident = cmm_clone_string(val);
    yylval.nodes          = NULL;
    yylval.len            = 0;
    yylval.text           = cmm_clone_string(yytext);

    cmm_log_node(&yylval);
}

CMM_AST_NODE cmm_node_tree(char* name, int len, ...) {
    va_list args;
    va_start(args, len);

    CMM_AST_NODE ret;
    ret.kind               = CMM_AST_NODE_TREE;
    ret.data.val_tree_name = name;
    ret.len                = len;
    ret.nodes              = malloc(sizeof(CMM_AST_NODE) * len);

    for (int i = 0; i < len; i++) ret.nodes[i] = va_arg(args, CMM_AST_NODE);
    return ret;
}

CMM_AST_NODE cmm_empty_tree(char* name) {
    CMM_AST_NODE ret;
    ret.kind               = CMM_AST_NODE_TREE;
    ret.data.val_tree_name = name;
    ret.len                = 0;
    ret.nodes              = NULL;
    return ret;
}