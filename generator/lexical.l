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

0[0-7]*[8-9]+[0-9]*         {
                                char buffer[100];
                                sprintf(buffer, "In `%s`: 8 or 9 can't occur in oct numbers", yytext);
                                cmm_report_error('A', buffer);
                                return YYerror;
                            }

.                           { 
                                char buffer[50];
                                sprintf(buffer, "unexpected pattern `%s`", yytext);
                                cmm_report_error('A', buffer);
                                return YYerror;
                            }

%%