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
[0-9]+\.[0-9]+              {
                                cmm_send_yylval_float(atof(yytext));
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return FLOAT;
                            }
0[0-7]*                     {
                                cmm_send_yylval_int(cmm_parse_int(yytext));
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return INT;
                            }
[1-9][0-9]*                 {
                                cmm_send_yylval_int(cmm_parse_int(yytext));
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return INT;
                            }
0[xX][0-9A-Fa-f]+           {
                                cmm_send_yylval_int(cmm_parse_int(yytext));
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return INT;
                            }
";"                         {
                                cmm_send_yylval_token(CMM_TK_SEMI);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return SEMI;
                            }
","                         {
                                cmm_send_yylval_token(CMM_TK_COMMA);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return COMMA;
                            }
"="                         {
                                cmm_send_yylval_token(CMM_TK_ASSIGNOP);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return ASSIGNOP;
                            }
">"|"<"|">="|"<="|"=="|"!=" {
                                cmm_send_yylval_token(CMM_TK_RELOP);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return RELOP;
                            }
"+"                         {
                                cmm_send_yylval_token(CMM_TK_PLUS);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return PLUS;
                            }
"-"                         {
                                cmm_send_yylval_token(CMM_TK_MINUS);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return MINUS;
                            }
"*"                         {
                                cmm_send_yylval_token(CMM_TK_STAR);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return STAR;
                            }
"/"                         {
                                cmm_send_yylval_token(CMM_TK_DIV);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return DIV;
                            }
"&&"                        {
                                cmm_send_yylval_token(CMM_TK_AND);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return AND;
                            }
"||"                        {
                                cmm_send_yylval_token(CMM_TK_OR);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return OR;
                            }
"."                         {
                                cmm_send_yylval_token(CMM_TK_DOT);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return DOT;
                            }
"!"                         {
                                cmm_send_yylval_token(CMM_TK_NOT);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return NOT;
                            }
"int"|"float"               {
                                cmm_send_yylval_type(yytext);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return TYPE;
                            }
"("                         {
                                cmm_send_yylval_token(CMM_TK_LP);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return LP;
                            }
")"                         {
                                cmm_send_yylval_token(CMM_TK_RP);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return RP;
                            }
"["                         {
                                cmm_send_yylval_token(CMM_TK_LB);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return LB;
                            }
"]"                         {
                                cmm_send_yylval_token(CMM_TK_RB);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return RB;
                            }
"{"                         {
                                cmm_send_yylval_token(CMM_TK_LC);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return LC;
                            }
"}"                         {
                                cmm_send_yylval_token(CMM_TK_RC);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return RC;
                            }
"struct"                    {
                                cmm_send_yylval_token(CMM_TK_STRUCT);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return STRUCT;
                            }
"return"                    {
                                cmm_send_yylval_token(CMM_TK_RETURN);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return RETURN;
                            }
"if"                        {
                                cmm_send_yylval_token(CMM_TK_IF);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return IF;
                            }
"else"                      {
                                cmm_send_yylval_token(CMM_TK_ELSE);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return ELSE;
                            }
"while"                     {
                                cmm_send_yylval_token(CMM_TK_WHILE);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return WHILE;
                            }
[a-zA-Z_][a-zA-Z0-9_]*      {
                                cmm_send_yylval_ident(yytext);
                                cmm_send_yylval_loc(yylineno, yycolumn);
                                return ID;
                            }



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
