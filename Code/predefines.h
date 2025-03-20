#ifndef LINCA_BYYL_PREDEFINES
#define LINCA_BYYL_PREDEFINES

#include <stdarg.h>
#include <stdio.h>

// #define CMM_DEBUG_FLAG
#define CMM_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CMM_MAX(a, b) ((a) > (b) ? (a) : (b))

extern int yylex(void);
extern int fileno(FILE*);

void yyerror(char* msg);

char* cmm_clone_string(char* str);

enum CMM_AST_NODE_KIND {
    CMM_AST_NODE_TOKEN,
    CMM_AST_NODE_INT,
    CMM_AST_NODE_FLOAT,
    CMM_AST_NODE_TYPE,
    CMM_AST_NODE_IDENT,
    CMM_AST_NODE_TREE,
};

typedef struct CMM_AST_LOCATION {
    int   line;
    int   column;
    int   end_line;
    int   end_column;
    char* text;
} CMM_AST_LOCATION;

typedef struct CMM_AST_NODE {
    enum CMM_AST_NODE_KIND kind;
    union CMM_AST_NODE_VAL {
        char* val_token;
        int   val_int;
        float val_float;
        char* val_type;
        char* val_ident;
        char* val_tree_name;
    } data;
    struct CMM_AST_NODE*    nodes;
    int                     len;
    struct CMM_AST_LOCATION location;
} CMM_AST_NODE;

#define YYSTYPE CMM_AST_NODE

extern YYSTYPE yylval;
extern char*   yytext;

void         cmm_report_error(char type, char* msg);
void         cmm_log_node(CMM_AST_NODE* val);
void         cmm_send_yylval_token(char* token_kind);
void         cmm_send_yylval_int(int val);
void         cmm_send_yylval_float(float val);
void         cmm_send_yylval_type(char* val);
void         cmm_send_yylval_ident(char* val);
void         cmm_send_yylval_loc(int, int);
int          cmm_parse_int(char*);
CMM_AST_NODE cmm_node_tree(char* name, int len, ...);
CMM_AST_NODE cmm_empty_tree(char* name);
#endif