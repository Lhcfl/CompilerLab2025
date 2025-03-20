%{
    #include "predefines.h"
    #include "syntax.tab.h"
%}

%option yylineno
%option noyywrap

%{
int yycolumn = 1;
int yy_last_lineno = -1;
#define YY_USER_ACTION yycolumn = yy_last_lineno != yylineno ? (yy_last_lineno = yylineno, 0) : yycolumn + yyleng;
%}

%%

[ \t]+                      ;
[\r\n]+                     ;

.                           { 
                                printf("\nerror: unexpected yytext = %s", yytext); 
                                printf("\ncode: %d", yytext[0]); 
                                printf("\n"); 
                            }

%%