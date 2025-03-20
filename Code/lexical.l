%{
    #include "predefines.h"
    #include "syntax.tab.h"
%}

%%

";"                         {
                                cmm_send_yylval_token(YYSYMBOL_SEMI);
                                return SEMI;
                            }
","                         {
                                cmm_send_yylval_token(YYSYMBOL_COMMA);
                                return COMMA;
                            }
"="                         {
                                cmm_send_yylval_token(YYSYMBOL_ASSIGNOP);
                                return ASSIGNOP;
                            }
">"|"<"|">="|"<="|"=="|"!=" {
                                cmm_send_yylval_token(YYSYMBOL_RELOP);
                                return RELOP;
                            }
"+"                         {
                                cmm_send_yylval_token(YYSYMBOL_PLUS);
                                return PLUS;
                            }
"-"                         {
                                cmm_send_yylval_token(YYSYMBOL_MINUS);
                                return MINUS;
                            }
"*"                         {
                                cmm_send_yylval_token(YYSYMBOL_STAR);
                                return STAR;
                            }
"/"                         {
                                cmm_send_yylval_token(YYSYMBOL_DIV);
                                return DIV;
                            }
"&&"                        {
                                cmm_send_yylval_token(YYSYMBOL_AND);
                                return AND;
                            }
"||"                        {
                                cmm_send_yylval_token(YYSYMBOL_OR);
                                return OR;
                            }
"."                         {
                                cmm_send_yylval_token(YYSYMBOL_DOT);
                                return DOT;
                            }
"!"                         {
                                cmm_send_yylval_token(YYSYMBOL_NOT);
                                return NOT;
                            }
"int"|"float"               { 
                                cmm_send_yylval_type(yytext);
                                return TYPE;
                            }
"("                         {
                                cmm_send_yylval_token(YYSYMBOL_LP);
                                return LP;
                            }
")"                         {
                                cmm_send_yylval_token(YYSYMBOL_RP);
                                return RP;
                            }
"["                         {
                                cmm_send_yylval_token(YYSYMBOL_LB);
                                return LB;
                            }
"]"                         {
                                cmm_send_yylval_token(YYSYMBOL_RB);
                                return RB;
                            }
"{"                         {
                                cmm_send_yylval_token(YYSYMBOL_LC);
                                return LC;
                            }
"}"                         {
                                cmm_send_yylval_token(YYSYMBOL_RC);
                                return RC;
                            }
"struct"                    {
                                cmm_send_yylval_token(YYSYMBOL_STRUCT);
                                return STRUCT;
                            }
"return"                    {
                                cmm_send_yylval_token(YYSYMBOL_RETURN);
                                return RETURN;
                            }
"if"                        {
                                cmm_send_yylval_token(YYSYMBOL_IF);
                                return IF;
                            }
"else"                      {
                                cmm_send_yylval_token(YYSYMBOL_ELSE);
                                return ELSE;
                            }
"while"                     {
                                cmm_send_yylval_token(YYSYMBOL_WHILE);
                                return WHILE;
                            }

[0-9]+\.[0-9]+              {
                                cmm_send_yylval_float(atof(yytext));
                                return FLOAT;
                            }

[0-9]+                      {
                                cmm_send_yylval_int(atoi(yytext));
                                return INT;
                            }

[a-zA-Z_][a-zA-Z0-9_]*      {
                                cmm_send_yylval_ident(yytext);
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