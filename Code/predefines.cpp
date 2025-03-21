#include "predefines.h"
#include "globals.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern int     yylex(void);
extern int     fileno(FILE*);
extern YYSTYPE yylval;
extern char*   yytext;
extern int     yyleng;
extern int     yylineno;

char error_buffer[200];

void yyerror(char* msg) { cmm_report_error('B', msg); }

void cmm_cancel_next_yyerror(int lineno) { cmm_cancel_next_error = lineno; }

void cmm_report_error(char type, char* msg) {
    if (type == 'A') { cmm_lexical_error += 1; }
    if (type == 'B') { cmm_syntax_error += 1; }
    if (cmm_cancel_next_error == yylineno && type == 'B') { return; }

    printf("Error type %c at Line %d: %s\n", type, yylineno, msg);
}

char* cmm_clone_string(char* str) {
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

void cmm_log_node(CMM_AST_NODE* val) {
#ifndef CMM_DEBUG_FLAG
    return;
#endif
    printf("{ \"line\": %d, \"col\": %d, \"end_line\": %d, \"end_col\": %d, ",
           val->location.line,
           val->location.column,
           val->location.end_line,
           val->location.end_column);
    switch (val->kind) {
        case CMM_AST_NODE_TOKEN: {
            printf("\"kind\": \"token\", \"val\": \"%s\", \"text\": \"%s\" }",
                   val->data.val_token,
                   val->location.text);
            break;
        }
        case CMM_AST_NODE_INT: {
            printf("\"kind\": \"int\", \"val\": %d }", val->data.val_int);
            break;
        }
        case CMM_AST_NODE_FLOAT: {
            printf("\"kind\": \"float\", \"val\": %f }", val->data.val_float);
            break;
        }
        case CMM_AST_NODE_TYPE: {
            printf("\"kind\": \"type\", \"val\": \"%s\" }", val->data.val_type);
            break;
        }
        case CMM_AST_NODE_IDENT: {
            printf("\"kind\": \"ident\", \"val\": \"%s\" }", val->data.val_ident);
            break;
        }
        case CMM_AST_NODE_TREE: {
            printf("\"kind\": \"tree\", \"val\": \"%s\", \"child\": [ ",
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

int cmm_parse_int(char* str) { return strtol(str, NULL, 0); }

void cmm_send_yylval_token(char* token_kind) {
    yylval.kind           = CMM_AST_NODE_TOKEN;
    yylval.data.val_token = token_kind;
    yylval.nodes          = NULL;
    yylval.len            = 0;
    yylval.location.text  = cmm_clone_string(yytext);

    cmm_log_node(&yylval);
}

void cmm_send_yylval_int(int val) {
    yylval.kind          = CMM_AST_NODE_INT;
    yylval.data.val_int  = val;
    yylval.nodes         = NULL;
    yylval.len           = 0;
    yylval.location.text = cmm_clone_string(yytext);

    cmm_log_node(&yylval);
}

void cmm_send_yylval_float(float val) {
    yylval.kind           = CMM_AST_NODE_FLOAT;
    yylval.data.val_float = val;
    yylval.nodes          = NULL;
    yylval.len            = 0;
    yylval.location.text  = cmm_clone_string(yytext);

    cmm_log_node(&yylval);
}

void cmm_send_yylval_type(char* val) {
    yylval.kind          = CMM_AST_NODE_TYPE;
    yylval.data.val_type = cmm_clone_string(val);
    yylval.nodes         = NULL;
    yylval.len           = 0;
    yylval.location.text = cmm_clone_string(yytext);

    cmm_log_node(&yylval);
}

void cmm_send_yylval_ident(char* val) {
    yylval.kind           = CMM_AST_NODE_IDENT;
    yylval.data.val_ident = cmm_clone_string(val);
    yylval.nodes          = NULL;
    yylval.len            = 0;
    yylval.location.text  = cmm_clone_string(yytext);

    cmm_log_node(&yylval);
}

void cmm_send_yylval_loc(int line, int column) {
    yylval.location.line       = line;
    yylval.location.column     = column;
    yylval.location.end_line   = line;
    yylval.location.end_column = column + yyleng;
}

CMM_AST_NODE cmm_node_tree(char* name, int len, ...) {
    va_list args;
    va_start(args, len);

    CMM_AST_NODE ret;
    ret.kind                = CMM_AST_NODE_TREE;
    ret.data.val_tree_name  = name;
    ret.len                 = len;
    ret.nodes               = malloc(sizeof(CMM_AST_NODE) * len);
    ret.location.line       = 0x7fffffff;
    ret.location.end_line   = -1;
    ret.location.column     = 0x7fffffff;
    ret.location.end_column = -1;

    for (int i = 0; i < len; i++) {
        CMM_AST_NODE     node   = va_arg(args, CMM_AST_NODE);
        CMM_AST_LOCATION loc    = node.location;
        ret.nodes[i]            = node;
        ret.location.line       = CMM_MIN(loc.line, ret.location.line);
        ret.location.column     = CMM_MIN(loc.column, ret.location.column);
        ret.location.end_line   = CMM_MAX(loc.end_line, ret.location.end_line);
        ret.location.end_column = CMM_MAX(loc.end_column, ret.location.end_column);
    }
    for (int i = 1; i < len; i++) {
        if (ret.nodes[i].location.line == 0x7fffffff) {
            ret.nodes[i].location.line     = ret.nodes[i - 1].location.end_line;
            ret.nodes[i].location.end_line = ret.nodes[i - 1].location.end_line;
        }
        if (ret.nodes[i].location.column == 0x7fffffff) {
            ret.nodes[i].location.end_column = ret.nodes[i - 1].location.end_column;
        }
    }

    return ret;
}

CMM_AST_NODE cmm_empty_tree(char* name) {
    CMM_AST_NODE ret;
    ret.kind                = CMM_AST_NODE_TREE;
    ret.data.val_tree_name  = name;
    ret.len                 = 0;
    ret.nodes               = NULL;
    ret.location.line       = 0x7fffffff;
    ret.location.end_line   = -1;
    ret.location.column     = 0x7fffffff;
    ret.location.end_column = -1;
    return ret;
}