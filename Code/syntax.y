%{
#include "predefines.h"
#include "globals.h"
%}

%token INT;
%token FLOAT;
%token ID;
%token SEMI
%token COMMA
%token ASSIGNOP
%token RELOP
%token PLUS
%token MINUS
%token STAR
%token DIV
%token AND
%token OR
%token DOT
%token NOT
%token TYPE
%token LP
%token RP
%token LB
%token RB
%token LC
%token RC
%token STRUCT
%token RETURN
%token IF
%token ELSE
%token WHILE

%right ASSIGNOP   // level 8
%left OR // level 7
%left AND // level 6
%left RELOP // level 5
%left PLUS MINUS // level 4
%left STAR DIV // level 3
%right NOT // level 2


%%


/* High level Definations */
Program: ExtDefList                            { $$ = cmm_node_tree(CMM_TK_Program, 1, $1); cmm_parsed_root = $$; }
    ;

ExtDefList: /* empty */                        { $$ = cmm_empty_tree(CMM_TK_ExtDefList); }
    | ExtDef ExtDefList                        { $$ = cmm_node_tree(CMM_TK_ExtDefList, 2, $1, $2); }
    ;

ExtDef: Specifier ExtDecList SEMI              { $$ = cmm_node_tree(CMM_TK_ExtDef, 3, $1, $2, $3); }
    | Specifier SEMI                           { $$ = cmm_node_tree(CMM_TK_ExtDef, 2, $1, $2); }
    | Specifier FunDec CompSt                  { $$ = cmm_node_tree(CMM_TK_ExtDef, 3, $1, $2, $3); }
    ;

ExtDecList: VarDec                             { $$ = cmm_node_tree(CMM_TK_ExtDecList, 1, $1); }
    | VarDec COMMA ExtDecList                  { $$ = cmm_node_tree(CMM_TK_ExtDecList, 3, $1, $2, $3); }
    ;

/* Specifiers */ 

Specifier: TYPE                                { $$ = cmm_node_tree(CMM_TK_Specifier, 1, $1); }
    | StructSpecifier                          { $$ = cmm_node_tree(CMM_TK_Specifier, 1, $1); }
    ;

StructSpecifier: STRUCT OptTag LC DefList RC   { $$ = cmm_node_tree(CMM_TK_StructSpecifier, 5, $1, $2, $3, $4, $5); }
    | STRUCT Tag                               { $$ = cmm_node_tree(CMM_TK_StructSpecifier, 2, $1, $2); }
    ;

OptTag: /* empty */                            { $$ = cmm_empty_tree(CMM_TK_OptTag); }
    | ID                                       { $$ = cmm_node_tree(CMM_TK_OptTag, 1, $1); }
    ;

Tag: ID                                        { $$ = cmm_node_tree(CMM_TK_Tag, 1, $1); }
    ;

/* Declarators */

VarDec: ID                                     { $$ = cmm_node_tree(CMM_TK_VarDec, 1, $1); }
    | VarDec LB INT RB                         { $$ = cmm_node_tree(CMM_TK_VarDec, 4, $1, $2, $3, $4); }
    ;

FunDec: ID LP VarList RP                       { $$ = cmm_node_tree(CMM_TK_FunDec, 4, $1, $2, $3, $4); }
    | ID LP RP                                 { $$ = cmm_node_tree(CMM_TK_FunDec, 3, $1, $2, $3); }
    ;

VarList: ParamDec COMMA VarList                { $$ = cmm_node_tree(CMM_TK_VarList, 3, $1, $2, $3); }
    | ParamDec                                 { $$ = cmm_node_tree(CMM_TK_VarList, 1, $1); }
    ;

ParamDec: Specifier VarDec                     { $$ = cmm_node_tree(CMM_TK_ParamDec, 2, $1, $2); }
    | error                                    { $$ = cmm_empty_tree(CMM_TK_ParamDec); }
    ;

/* Statements */
CompSt: LC DefList StmtList RC                 { $$ = cmm_node_tree(CMM_TK_CompSt, 4, $1, $2, $3, $4); }
    | error RC                                 { $$ = cmm_node_tree(CMM_TK_CompSt, 1, $1); }
    ;

StmtList: /* empty */                          { $$ = cmm_empty_tree(CMM_TK_StmtList); }
    | Stmt StmtList                            { $$ = cmm_node_tree(CMM_TK_StmtList, 2, $1, $2); }
    ;

Stmt: Exp SEMI                                 { $$ = cmm_node_tree(CMM_TK_Stmt, 2, $1, $2); }
    | CompSt                                   { $$ = cmm_node_tree(CMM_TK_Stmt, 1, $1); }
    | RETURN Exp SEMI                          { $$ = cmm_node_tree(CMM_TK_Stmt, 3, $1, $2, $3); }
    | IF LP Exp RP Stmt                        { $$ = cmm_node_tree(CMM_TK_Stmt, 5, $1, $2, $3, $4, $5); }
    | IF LP error RP Stmt                      { $$ = cmm_node_tree(CMM_TK_Stmt, 4, $1, $2, $3, $4); }
    | IF LP Exp RP Stmt ELSE Stmt              { $$ = cmm_node_tree(CMM_TK_Stmt, 7, $1, $2, $3, $4, $5, $6, $7); }
    | IF LP error RP Stmt ELSE Stmt            { $$ = cmm_node_tree(CMM_TK_Stmt, 6, $1, $2, $3, $4, $5, $6); }
    | WHILE LP Exp RP Stmt                     { $$ = cmm_node_tree(CMM_TK_Stmt, 5, $1, $2, $3, $4, $5); }
    | WHILE LP error Stmt                      { $$ = cmm_node_tree(CMM_TK_Stmt, 3, $1, $2, $3); }
    | error Stmt                               { $$ = cmm_node_tree(CMM_TK_Stmt, 1, $1); }
    | error SEMI                               { $$ = cmm_node_tree(CMM_TK_Stmt, 1, $1); }
    ;

/** Local Definations */
DefList: /* empty */                           { $$ = cmm_empty_tree(CMM_TK_DefList); }
    | Def DefList                              { $$ = cmm_node_tree(CMM_TK_DefList, 2, $1, $2); }
    ;

Def: Specifier DecList SEMI                    { $$ = cmm_node_tree(CMM_TK_Def, 3, $1, $2, $3); }
    | error SEMI                               { $$ = cmm_node_tree(CMM_TK_Def, 1, $1); }
    ;

DecList: Dec                                   { $$ = cmm_node_tree(CMM_TK_DecList, 1, $1); }
    | Dec COMMA DecList                        { $$ = cmm_node_tree(CMM_TK_DecList, 3, $1, $2, $3); }
    ;

Dec: VarDec                                    { $$ = cmm_node_tree(CMM_TK_Dec, 1, $1); }
    | VarDec ASSIGNOP Exp                      { $$ = cmm_node_tree(CMM_TK_Dec, 3, $1, $2, $3); }
    ;

/** expressions */

Exp: Exp ASSIGNOP Exp                          { $$ = cmm_node_tree(CMM_TK_Exp, 3, $1, $2, $3); }
    | Exp OR Exp                               { $$ = cmm_node_tree(CMM_TK_Exp, 3, $1, $2, $3); }
    | Exp AND Exp                              { $$ = cmm_node_tree(CMM_TK_Exp, 3, $1, $2, $3); }
    | Exp RELOP Exp                            { $$ = cmm_node_tree(CMM_TK_Exp, 3, $1, $2, $3); }
    | Exp PLUS Exp                             { $$ = cmm_node_tree(CMM_TK_Exp, 3, $1, $2, $3); }
    | Exp MINUS Exp                            { $$ = cmm_node_tree(CMM_TK_Exp, 3, $1, $2, $3); }
    | Exp STAR Exp                             { $$ = cmm_node_tree(CMM_TK_Exp, 3, $1, $2, $3); }
    | Exp DIV Exp                              { $$ = cmm_node_tree(CMM_TK_Exp, 3, $1, $2, $3); }
    | MINUS Exp                                { $$ = cmm_node_tree(CMM_TK_Exp, 2, $1, $2); }
    | NOT Exp                                  { $$ = cmm_node_tree(CMM_TK_Exp, 2, $1, $2); }
    | LP Exp RP                                { $$ = cmm_node_tree(CMM_TK_Exp, 3, $1, $2, $3); }
    | ID LP Args RP                            { $$ = cmm_node_tree(CMM_TK_Exp, 4, $1, $2, $3, $4); }
    | ID LP RP                                 { $$ = cmm_node_tree(CMM_TK_Exp, 3, $1, $2, $3); }
    | Exp LB Exp RB                            { $$ = cmm_node_tree(CMM_TK_Exp, 4, $1, $2, $3, $4); }
    | Exp DOT ID                               { $$ = cmm_node_tree(CMM_TK_Exp, 3, $1, $2, $3); }
    | ID                                       { $$ = cmm_node_tree(CMM_TK_Exp, 1, $1); }
    | INT                                      { $$ = cmm_node_tree(CMM_TK_Exp, 1, $1); }
    | FLOAT                                    { $$ = cmm_node_tree(CMM_TK_Exp, 1, $1); }
    | error RP                                 { $$ = cmm_node_tree(CMM_TK_Exp, 1, $1); }
    | error                                    { $$ = cmm_empty_tree(CMM_TK_Exp); }
    ;

Args: Exp COMMA Args                           { $$ = cmm_node_tree(CMM_TK_Args, 3, $1, $2, $3); }
    | Exp                                      { $$ = cmm_node_tree(CMM_TK_Args, 1, $1); }
    ;


%%


#include "lex.yy.c"