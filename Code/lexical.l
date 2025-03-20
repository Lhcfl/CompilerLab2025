%{
    #include "predefines.h"
    #include "syntax.tab.h"
%}

%option yylineno
%option noyywrap

%%

";"                         {
                                cmm_send_yylval_token("SEMI");
                                cmm_send_yylval_loc(yylineno, 1);
                                return SEMI;
                            }
","                         {
                                cmm_send_yylval_token("COMMA");
                                cmm_send_yylval_loc(yylineno, 1);
                                return COMMA;
                            }
"="                         {
                                cmm_send_yylval_token("ASSIGNOP");
                                cmm_send_yylval_loc(yylineno, 1);
                                return ASSIGNOP;
                            }
">"|"<"|">="|"<="|"=="|"!=" {
                                cmm_send_yylval_token("RELOP");
                                cmm_send_yylval_loc(yylineno, 1);
                                return RELOP;
                            }
"+"                         {
                                cmm_send_yylval_token("PLUS");
                                cmm_send_yylval_loc(yylineno, 1);
                                return PLUS;
                            }
"-"                         {
                                cmm_send_yylval_token("MINUS");
                                cmm_send_yylval_loc(yylineno, 1);
                                return MINUS;
                            }
"*"                         {
                                cmm_send_yylval_token("STAR");
                                cmm_send_yylval_loc(yylineno, 1);
                                return STAR;
                            }
"/"                         {
                                cmm_send_yylval_token("DIV");
                                cmm_send_yylval_loc(yylineno, 1);
                                return DIV;
                            }
"&&"                        {
                                cmm_send_yylval_token("AND");
                                cmm_send_yylval_loc(yylineno, 1);
                                return AND;
                            }
"||"                        {
                                cmm_send_yylval_token("OR");
                                cmm_send_yylval_loc(yylineno, 1);
                                return OR;
                            }
"."                         {
                                cmm_send_yylval_token("DOT");
                                cmm_send_yylval_loc(yylineno, 1);
                                return DOT;
                            }
"!"                         {
                                cmm_send_yylval_token("NOT");
                                cmm_send_yylval_loc(yylineno, 1);
                                return NOT;
                            }
"int"|"float"               { 
                                cmm_send_yylval_type(yytext);
                                cmm_send_yylval_loc(yylineno, 1);
                                return TYPE;
                            }
"("                         {
                                cmm_send_yylval_token("LP");
                                cmm_send_yylval_loc(yylineno, 1);
                                return LP;
                            }
")"                         {
                                cmm_send_yylval_token("RP");
                                cmm_send_yylval_loc(yylineno, 1);
                                return RP;
                            }
"["                         {
                                cmm_send_yylval_token("LB");
                                cmm_send_yylval_loc(yylineno, 1);
                                return LB;
                            }
"]"                         {
                                cmm_send_yylval_token("RB");
                                cmm_send_yylval_loc(yylineno, 1);
                                return RB;
                            }
"{"                         {
                                cmm_send_yylval_token("LC");
                                cmm_send_yylval_loc(yylineno, 1);
                                return LC;
                            }
"}"                         {
                                cmm_send_yylval_token("RC");
                                cmm_send_yylval_loc(yylineno, 1);
                                return RC;
                            }
"struct"                    {
                                cmm_send_yylval_token("STRUCT");
                                cmm_send_yylval_loc(yylineno, 1);
                                return STRUCT;
                            }
"return"                    {
                                cmm_send_yylval_token("RETURN");
                                cmm_send_yylval_loc(yylineno, 1);
                                return RETURN;
                            }
"if"                        {
                                cmm_send_yylval_token("IF");
                                cmm_send_yylval_loc(yylineno, 1);
                                return IF;
                            }
"else"                      {
                                cmm_send_yylval_token("ELSE");
                                cmm_send_yylval_loc(yylineno, 1);
                                return ELSE;
                            }
"while"                     {
                                cmm_send_yylval_token("WHILE");
                                cmm_send_yylval_loc(yylineno, 1);
                                return WHILE;
                            }

[0-9]+\.[0-9]+              {
                                cmm_send_yylval_float(atof(yytext));
                                cmm_send_yylval_loc(yylineno, 1);
                                return FLOAT;
                            }

[0-9]+                      {
                                cmm_send_yylval_int(atoi(yytext));
                                cmm_send_yylval_loc(yylineno, 1);
                                return INT;
                            }

[a-zA-Z_][a-zA-Z0-9_]*      {
                                cmm_send_yylval_ident(yytext);
                                cmm_send_yylval_loc(yylineno, 1);
                                return ID;
                            }

[ \t]+                      ;
[\r\n]+                     ;

.                           { 
                                printf("\nerror: unexpected yytext = %s", yytext); 
                                printf("\ncode: %d", yytext[0]); 
                                printf("\n"); 
                            }

%%