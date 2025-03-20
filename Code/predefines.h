#ifndef LINCA_BYYL_PREDEFINES
#define LINCA_BYYL_PREDEFINES

#include <stdarg.h>
#include <stdio.h>

#define CMM_DEBUG_FLAG

extern int yylex(void);
extern int fileno(FILE*);

void yyerror(char* msg);

char* cmm_clone_string(const char* str);

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
        char* val_tree_name;
    } data;
    struct CMM_AST_NODE* nodes;
    int                  len;
    char*                text;
} CMM_AST_NODE;

#define YYSTYPE CMM_AST_NODE

extern YYSTYPE yylval;
extern char*   yytext;

void          cmm_log_node(CMM_AST_NODE* val);
void          cmm_send_yylval_token(int yysymbol_kind);
void          cmm_send_yylval_int(int val);
void          cmm_send_yylval_float(float val);
void          cmm_send_yylval_type(char* val);
void          cmm_send_yylval_ident(char* val);
CMM_AST_NODE  cmm_node_tree(char* name, int len, ...);
CMM_AST_NODE  cmm_empty_tree(char* name);
void          cmm_set_result(CMM_AST_NODE node);
CMM_AST_NODE* cmm_get_result();
#endif