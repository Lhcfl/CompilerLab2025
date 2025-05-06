#include "predefines.h"
#include "globals.h"
#include "syndef.h"
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

void cmm_free_ast(CMM_AST_NODE* root) {
    free(root->location.text);
    switch (root->kind) {
        case CMM_AST_NODE_TOKEN: {
            if (root->token == CMM_TK_RELOP) { free(root->data.val_relop); }
            break;
        }
        case CMM_AST_NODE_INT: {
            break;
        }
        case CMM_AST_NODE_FLOAT: {
            break;
        }
        case CMM_AST_NODE_TYPE: {
            free(root->data.val_type);
            break;
        }
        case CMM_AST_NODE_IDENT: {
            free(root->data.val_ident);
            break;
        }
        case CMM_AST_NODE_TREE: {
            for (int i = 0; i < root->len; i++) {
                cmm_free_ast(root->nodes + i);
            }
            free(root->nodes);
            break;
        }
    }
}

/// NULL SAFE
char* cmm_clone_string(const char* str) {
    if (str == NULL) return NULL;
    size_t len   = strlen(str);
    char*  clone = (char*)malloc((len + 1) * sizeof(char));
    if (clone == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    strcpy(clone, str);
    return clone;
}

/// allocate a new string which is the concatenation of n strings
char* cmm_concat_string(int len, ...) {
    va_list args;

    va_start(args, len);
    size_t total_len = 0;
    for (int i = 0; i < len; i++) {
        char* str = va_arg(args, char*);
        total_len += strlen(str);
    }
    va_end(args);

    char* result = (char*)malloc((total_len + 1) * sizeof(char));
    if (result == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    va_start(args, len);
    size_t offset = 0;
    for (int i = 0; i < len; i++) {
        char*  str     = va_arg(args, char*);
        size_t str_len = strlen(str);
        memcpy(result + offset, str, str_len);
        offset += str_len;
    }
    result[total_len] = '\0';
    va_end(args);

    return result;
}

#define YYSTYPE CMM_AST_NODE

void cmm_log_node(CMM_AST_NODE* val) {
    printf("{ \"line\": %d, \"col\": %d, \"end_line\": %d, \"end_col\": %d, ",
           val->location.line,
           val->location.column,
           val->location.end_line,
           val->location.end_column);
    switch (val->kind) {
        case CMM_AST_NODE_TOKEN: {
            printf("\"kind\": \"token\", \"val\": \"%s\", \"text\": \"%s\" }",
                   cmm_token_tostring(val->token),
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
            printf("\"kind\": \"ident\", \"val\": \"%s\" }",
                   val->data.val_ident);
            break;
        }
        case CMM_AST_NODE_TREE: {
            printf("\"kind\": \"tree\", \"val\": \"%s\", \"child\": [ ",
                   cmm_token_tostring(val->token));
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

CMM_AST_NODE make_default_ast() {
    CMM_AST_NODE ret;
    ret.len                 = 0;
    ret.nodes               = NULL;
    ret.location.line       = 0x7fffffff;
    ret.location.end_line   = -1;
    ret.location.column     = 0x7fffffff;
    ret.location.end_column = -1;
    ret.context.kind        = CMM_AST_KIND_UNDEFINED;
    return ret;
}

void cmm_send_yylval_token(enum CMM_SYNTAX_TOKEN token) {
    yylval               = make_default_ast();
    yylval.kind          = CMM_AST_NODE_TOKEN;
    yylval.token         = token;
    yylval.location.text = cmm_clone_string(yytext);

    if (token == CMM_TK_RELOP) {
        yylval.data.val_relop = cmm_clone_string(yytext);
    }
}

void cmm_send_yylval_int(int val) {
    yylval               = make_default_ast();
    yylval.kind          = CMM_AST_NODE_INT;
    yylval.token         = CMM_TK_INT;
    yylval.data.val_int  = val;
    yylval.location.text = cmm_clone_string(yytext);
}

void cmm_send_yylval_float(float val) {
    yylval                = make_default_ast();
    yylval.kind           = CMM_AST_NODE_FLOAT;
    yylval.token          = CMM_TK_FLOAT;
    yylval.data.val_float = val;
    yylval.location.text  = cmm_clone_string(yytext);
}

void cmm_send_yylval_type(char* val) {
    yylval               = make_default_ast();
    yylval.kind          = CMM_AST_NODE_TYPE;
    yylval.token         = CMM_TK_TYPE;
    yylval.data.val_type = cmm_clone_string(val);
    yylval.location.text = cmm_clone_string(yytext);
}

void cmm_send_yylval_ident(char* val) {
    yylval                = make_default_ast();
    yylval.kind           = CMM_AST_NODE_IDENT;
    yylval.token          = CMM_TK_ID;
    yylval.data.val_ident = cmm_clone_string(val);
    yylval.location.text  = cmm_clone_string(yytext);
}

void cmm_send_yylval_loc(int line, int column) {
    yylval.location.line       = line;
    yylval.location.column     = column;
    yylval.location.end_line   = line;
    yylval.location.end_column = column + yyleng;
}

CMM_AST_NODE cmm_node_tree(enum CMM_SYNTAX_TOKEN name, int len, ...) {
    va_list args;
    va_start(args, len);

    CMM_AST_NODE ret = make_default_ast();
    ret.kind         = CMM_AST_NODE_TREE;
    ret.token        = name;
    ret.len          = len;
    ret.nodes        = malloc(sizeof(CMM_AST_NODE) * len);

    for (int i = 0; i < len; i++) {
        CMM_AST_NODE     node = va_arg(args, CMM_AST_NODE);
        CMM_AST_LOCATION loc  = node.location;
        ret.nodes[i]          = node;
        ret.location.line     = CMM_MIN(loc.line, ret.location.line);
        ret.location.column   = CMM_MIN(loc.column, ret.location.column);
        ret.location.end_line = CMM_MAX(loc.end_line, ret.location.end_line);
        ret.location.end_column =
            CMM_MAX(loc.end_column, ret.location.end_column);
    }
    for (int i = 1; i < len; i++) {
        if (ret.nodes[i].location.line == 0x7fffffff) {
            ret.nodes[i].location.line     = ret.nodes[i - 1].location.end_line;
            ret.nodes[i].location.end_line = ret.nodes[i - 1].location.end_line;
        }
        if (ret.nodes[i].location.column == 0x7fffffff) {
            ret.nodes[i].location.end_column =
                ret.nodes[i - 1].location.end_column;
        }
    }

    va_end(args);

    return ret;
}

CMM_AST_NODE cmm_empty_tree(enum CMM_SYNTAX_TOKEN name) {
    CMM_AST_NODE ret;
    ret.kind                = CMM_AST_NODE_TREE;
    ret.token               = name;
    ret.len                 = 0;
    ret.nodes               = NULL;
    ret.location.line       = 0x7fffffff;
    ret.location.end_line   = -1;
    ret.location.column     = 0x7fffffff;
    ret.location.end_column = -1;
    return ret;
}

char* gen_unnamed_struct_name() {
    static int unnamed_struct_count = 0;
    char*      name                 = malloc(50);
    sprintf(name, "(unnamed)struct_%d", unnamed_struct_count++);
    return name;
}

void cmm_debug_show_node_info(CMM_AST_NODE* val, int fuel) {
    if (fuel <= 0) return;
    if (val->kind == CMM_AST_NODE_TREE && val->len == 0) { return; }
    switch (val->kind) {
        case CMM_AST_NODE_TOKEN: {
            printf("%s", val->location.text);
            break;
        }
        case CMM_AST_NODE_INT: {
            printf("INT=%d", val->data.val_int);
            break;
        }
        case CMM_AST_NODE_FLOAT: {
            printf("FLOAT=%f", val->data.val_float);
            break;
        }
        case CMM_AST_NODE_TYPE: {
            printf("TYPE=%s", val->data.val_type);
            break;
        }
        case CMM_AST_NODE_IDENT: {
            printf("ID=%s", val->data.val_ident);
            break;
        }
        case CMM_AST_NODE_TREE: {
            printf("%s", cmm_token_tostring(val->token));
            if (fuel > 1) {
                printf("=[");
                for (int i = 0; i < val->len; i++) {
                    cmm_debug_show_node_info(val->nodes + i, fuel - 1);
                    if (i != val->len - 1) printf(" ");
                }
                printf("]");
            }
            break;
        }
    }
}