%{
#include "predefines.h"
%}

%union {
    CMM_AST_NODE node;
}

%token <node> INT;
%token <node> FLOAT;
%token <node> ID;
%token <node> SEMI
%token <node> COMMA
%token <node> ASSIGNOP
%token <node> RELOP
%token <node> PLUS
%token <node> MINUS
%token <node> STAR
%token <node> DIV
%token <node> AND
%token <node> OR
%token <node> DOT
%token <node> NOT
%token <node> TYPE
%token <node> LP
%token <node> RP
%token <node> LB
%token <node> RB
%token <node> LC
%token <node> RC
%token <node> STRUCT
%token <node> RETURN
%token <node> IF
%token <node> ELSE
%token <node> WHILE

%type <node> Program
%type <node> ExtDefList
%type <node> ExtDef
%type <node> Specifier
%type <node> ExtDecList
%type <node> FunDec
%type <node> CompSt
%type <node> VarDec
%type <node> StructSpecifier
%type <node> OptTag
%type <node> DefList
%type <node> Tag
%type <node> VarList
%type <node> ParamDec
%type <node> StmtList
%type <node> Stmt
%type <node> Exp
%type <node> Def
%type <node> DecList
%type <node> Dec
%type <node> Args

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