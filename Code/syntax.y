%{
    #include "predefines.h"
%}

%union {
    int type_int;
    float type_float;
    char* type_ident;
    char* type_type;
    int type_token;
}

%token <type_int>   INT;
%token <type_float> FLOAT;
%token <type_ident> ID;
%token <type_token> SEMI
%token <type_token> COMMA
%token <type_token> ASSIGNOP
%token <type_token> RELOP
%token <type_token> PLUS
%token <type_token> MINUS
%token <type_token> STAR
%token <type_token> DIV
%token <type_token> AND
%token <type_token> OR
%token <type_token> DOT
%token <type_token> NOT
%token <type_type>  TYPE
%token <type_token> LP
%token <type_token> RP
%token <type_token> LB
%token <type_token> RB
%token <type_token> LC
%token <type_token> RC
%token <type_token> STRUCT
%token <type_token> RETURN
%token <type_token> IF
%token <type_token> ELSE
%token <type_token> WHILE

%%

/* High level Definations */
Program: ExtDefList
    ;

ExtDefList: /* empty */
    | ExtDef ExtDefList
    ;

ExtDef: Specifier ExtDecList SEMI
    | Specifier SEMI
    | Specifier FunDec CompSt
    ;

ExtDecList: VarDec
    | VarDec COMMA ExtDecList
    ;

/* Specifiers */ 

Specifier: TYPE
    | StructSpecifier
    ;

StructSpecifier: STRUCT OptTag LC DefList RC
    | STRUCT Tag
    ;

OptTag: /* empty */ 
    | ID
    ;

Tag: ID
    ;

/* Declarators */

VarDec: ID
    | VarDec LB INT RB
    ;

FunDec: ID LP VarList RP
    | ID LP RP
    ;

VarList: ParamDec COMMA VarList
    | ParamDec
    ;

ParamDec: Specifier VarDec
    ;

/* Statements */
CompSt: LC DefList StmtList RC
    ;

StmtList: /* empty */
    | Stmt StmtList
    ;

Stmt: Exp SEMI
    | CompSt
    | RETURN Exp SEMI
    | IF LP Exp RP Stmt
    | IF LP Exp RP Stmt ELSE Stmt
    | WHILE LP Exp RP Stmt
    ;

/** Local Definations */
DefList: /* empty */
    | Def DefList
    ;

Def: Specifier DecList SEMI
    ;

DecList: Dec
    | Dec COMMA DecList
    ;

Dec: VarDec
    | VarDec ASSIGNOP Exp
    ;

/** expressions */

Exp: Exp ASSIGNOP Exp
    | Exp AND Exp
    | Exp OR Exp
    | Exp RELOP Exp
    | Exp PLUS Exp
    | Exp MINUS Exp
    | Exp STAR Exp
    | Exp DIV Exp
    | LP Exp RP
    | MINUS Exp
    | NOT Exp
    | ID LP Args RP
    | ID LP RP
    | Exp LB Exp RB
    | Exp DOT ID
    | ID
    | INT
    | FLOAT
    ;

Args: Exp COMMA Args
    | Exp
    ;
%%

#include "lex.yy.c"