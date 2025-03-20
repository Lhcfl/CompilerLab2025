#ifndef LINCA_BYYL_PREDEFINES
#define LINCA_BYYL_PREDEFINES

#include <stdio.h>

#define CMM_DEBUG_FLAG

extern int yylex(void);
extern int fileno(FILE*);

void yyerror(char* msg) { fprintf(stderr, "error: %s\n", msg); }

void cmm_log_ast(char* name) {
#ifndef CMM_DEBUG_FLAG
    return;
#endif
    printf("{ \"kind\": \"ast\", \"name\": \"%s\", \"val\": [] }, ", name);
}

enum CMM_AST_NODE_KIND {
    CMM_AST_NODE_TOKEN,
    CMM_AST_NODE_INT,
    CMM_AST_NODE_FLOAT,
    CMM_AST_NODE_TYPE,
    CMM_AST_NODE_IDENT,
    CMM_AST_NODE_TREE,
};

typedef struct CMM_AST_NODE {
    enum CMM_AST_NODE_KIND kind;
    union CMM_AST_NODE_VAL {
        int   val_token;
        int   val_int;
        float val_float;
        char* val_type;
        char* val_ident;
    } data;
    struct CMM_AST_NODE* nodes;
    int                  len;
    char*                text;
} CMM_AST_NODE;

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
            printf("{ \"kind\": \"tree\", \"val\": [ ");
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
    yylval.text           = yytext;

    cmm_log_node(&yylval);
}

void cmm_send_yylval_int(int val) {
    yylval.kind         = CMM_AST_NODE_INT;
    yylval.data.val_int = val;
    yylval.nodes        = NULL;
    yylval.len          = 0;
    yylval.text         = yytext;

    cmm_log_node(&yylval);
}

void cmm_send_yylval_float(float val) {
    yylval.kind           = CMM_AST_NODE_INT;
    yylval.data.val_float = val;
    yylval.nodes          = NULL;
    yylval.len            = 0;
    yylval.text           = yytext;

    cmm_log_node(&yylval);
}

void cmm_send_yylval_type(char* val) {
    yylval.kind          = CMM_AST_NODE_TYPE;
    yylval.data.val_type = val;
    yylval.nodes         = NULL;
    yylval.len           = 0;
    yylval.text          = yytext;

    cmm_log_node(&yylval);
}

void cmm_send_yylval_ident(char* val) {
    yylval.kind           = CMM_AST_NODE_IDENT;
    yylval.data.val_ident = val;
    yylval.nodes          = NULL;
    yylval.len            = 0;
    yylval.text           = yytext;

    cmm_log_node(&yylval);
}

#endif