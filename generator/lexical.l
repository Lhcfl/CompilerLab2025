%{
    #include "predefines.h"
    #include "syntax.tab.h"
%}

%option yylineno
%option noyywrap

%{
int yycolumn = 1;
int yy_last_lineno = -1;
#define YYUSER_ACTION \
    yycolumn = yy_last_lineno != yylineno ? 1 : yycolumn + yyleng; \
    yy_last_lineno = yylineno;
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