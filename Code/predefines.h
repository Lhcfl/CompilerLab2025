#ifndef LINCA_BYYL_PREDEFINES
#define LINCA_BYYL_PREDEFINES

#include <stdarg.h>
#include <stdio.h>
#include "syndef.h"

// #define CMM_DEBUG_LAB1
#define CMM_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CMM_MAX(a, b) ((a) > (b) ? (a) : (b))


#define cmm_panic(...)                               \
    {                                                \
        printf("\033[1;31m In %s (%s:%d) [PAINC]: ", \
               __func__,                             \
               __FILE__,                             \
               __LINE__);                            \
        printf(__VA_ARGS__);                         \
        printf("\033[0m\n");                         \
        exit(0);                                     \
    }

#define COLOR_EMPTY "\033[0m"
#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN "\033[1;36m"
#define COLOR_WHITE "\033[1;37m"

#define cmm_debug(color, ...) \
    {                         \
        printf(color);        \
        printf(__VA_ARGS__);  \
        printf(COLOR_EMPTY);  \
    }

extern int yylex(void);
extern int fileno(FILE*);

void yyerror(char* msg);

/// allocate a new string
char* cmm_clone_string(const char* str);
/// allocate a new string which is the concatenation of n strings
char* cmm_concat_string(int n, ...);

enum CMM_AST_NODE_KIND {
    CMM_AST_NODE_TOKEN,
    CMM_AST_NODE_INT,
    CMM_AST_NODE_FLOAT,
    CMM_AST_NODE_TYPE,
    CMM_AST_NODE_IDENT,
    CMM_AST_NODE_TREE,
};

/// 一个 AST 在语义上可能是什么
enum CMM_SEM_AST_KIND {
    /// 尚未分析
    CMM_AST_KIND_UNDEFINED,
    /// 类型的 context
    CMM_AST_KIND_TYPE,
    /// 变量的 context
    CMM_AST_KIND_VAR,
    /// Ident context
    CMM_AST_KIND_IDENT,
    /// 声明的 context
    CMM_AST_KIND_DECLARE,
    /// 表达式的 context
    CMM_AST_KIND_EXPR,
    /// 块的 context
    CMM_AST_KIND_BLOCK,
};

enum CMM_SEM_TYPE_KIND {
    /// 特殊的错误类型
    CMM_ERROR_TYPE,
    /// 原语类型，在这里是 int, float
    CMM_PRIMITIVE_TYPE,
    /// 积类型，或者说结构体类型
    CMM_PROD_TYPE,
    /// 数组类型
    CMM_ARRAY_TYPE,
    /// 函数类型
    CMM_FUNCTION_TYPE
};

enum CMM_VALUE_KIND {
    LVALUE,
    RVALUE,
};

typedef struct CMM_AST_LOCATION {
    int   line;
    int   column;
    int   end_line;
    int   end_column;
    char* text;
} CMM_AST_LOCATION;

/// 语义分析：类型
typedef struct CMM_SEM_TYPE {
    /// 类型的 kind
    enum CMM_SEM_TYPE_KIND kind;
    /// 类型的名字
    char*                  name;
    /// 类型的绑定名。仅限结构体可用。
    char*                  bind;
    /// 某种 Size，依Kind决定意义
    int                    size;
    /// 类型的“内部”。
    /// 对于原语，我们期望它是 NULL
    /// 对于结构体，我们期望它是 Array<CMM_SEM_TYPE>
    /// 对于数组，我们期望它是一个 CMM_SEM_TYPE
    /// 对于函数，我们期望它是 [Return Type, ...Args Type]
    struct CMM_SEM_TYPE*   inner;
    /// 占用的空间大小
    int                    bytes4;
} CMM_SEM_TYPE;

/// 语义分析的 Context
typedef struct CMM_SEM_CONTEXT {
    enum CMM_SEM_AST_KIND kind;
    enum CMM_VALUE_KIND   value_kind;
    struct {
        CMM_SEM_TYPE type;
        char*        ident;
    } data;
} CMM_SEM_CONTEXT;

typedef struct CMM_AST_NODE {
    enum CMM_AST_NODE_KIND kind;
    enum CMM_SYNTAX_TOKEN  token;
    union CMM_AST_NODE_VAL {
        int   val_int;
        float val_float;
        char* val_type;
        char* val_ident;
        char* val_relop;
    } data;
    struct CMM_AST_NODE*    nodes;
    int                     len;
    struct CMM_AST_LOCATION location;
    CMM_SEM_CONTEXT         context;
} CMM_AST_NODE;

#define YYSTYPE CMM_AST_NODE

extern YYSTYPE yylval;
extern char*   yytext;

void cmm_free_ast(CMM_AST_NODE* root);

void         cmm_cancel_next_yyerror(int);
void         cmm_report_error(char type, char* msg);
void         cmm_log_node(CMM_AST_NODE* val);
void         cmm_send_yylval_token(enum CMM_SYNTAX_TOKEN token);
void         cmm_send_yylval_int(int val);
void         cmm_send_yylval_float(float val);
void         cmm_send_yylval_type(char* val);
void         cmm_send_yylval_ident(char* val);
void         cmm_send_yylval_loc(int, int);
int          cmm_parse_int(char*);
CMM_AST_NODE cmm_node_tree(enum CMM_SYNTAX_TOKEN name, int len, ...);
CMM_AST_NODE cmm_empty_tree(enum CMM_SYNTAX_TOKEN name);
char*        gen_unnamed_struct_name();
void         cmm_debug_show_node_info(CMM_AST_NODE* val, int fuel);

#endif