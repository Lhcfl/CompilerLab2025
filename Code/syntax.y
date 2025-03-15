%{
    #include "predefines.h"
%}

%token INT
%token ADD SUB MUL DIV
%token LINEBREAK

%%

LineCalc: /* empty */
    | Calc LINEBREAK LineCalc
    ;

Calc: Exp { printf("= %d\n", $1); }
    ;

Exp : Factor     
    | Exp ADD Factor  { $$ = $1 + $3; }    
    | Exp SUB Factor  { $$ = $1 - $3; } 
    ; 
Factor : Term 
    | Factor MUL Term  { $$ = $1 * $3; }
    | Factor DIV Term  { $$ = $1 / $3; }
    ;

Term : INT { printf("[INT](%d) ", $1); $$ = $1; }
    ;

%%

#include "lex.yy.c"