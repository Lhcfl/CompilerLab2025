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

\/\/[^\n]*                  ;
[ \t]+                      ;
[\r\n]+                     ;

[0-9]*\.[0-9]*[eE][\+\-]?[0-9]+ {
                                char buffer[100];
                                sprintf(buffer, "In `%s`: bad float", yytext);
                                cmm_report_error('A', buffer);
                                cmm_send_yylval_float(0);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return FLOAT;
                            }

0[0-7]*[8-9]+[0-9]*         {
                                char buffer[100];
                                sprintf(buffer, "In `%s`: 8 or 9 can't occur in oct numbers", yytext);
                                cmm_report_error('A', buffer);
                                cmm_send_yylval_int(0);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return INT;
                            }

0[xX][0-9A-Fa-f]*[g-zG-Z]+[0-9A-Fa-f]*  {
                                char buffer[100];
                                sprintf(buffer, "In `%s`: G-Z can't occur in hex numbers", yytext);
                                cmm_report_error('A', buffer);
                                cmm_send_yylval_int(0);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return INT;
                            }

.                           { 
                                char buffer[100];
                                if (yytext[0] < 0) {
                                    sprintf(buffer, "unexpected pattern unknown");
                                } else {
                                    sprintf(buffer, "unexpected pattern `%s` (code %d)", yytext, yytext[0]);
                                }
                                cmm_report_error('A', buffer);
                                cmm_cancel_next_yyerror(yylineno);
                            }

%%