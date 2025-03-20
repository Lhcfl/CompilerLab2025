%{
    #include "predefines.h"
    #include "syntax.tab.h"
%}

%%

";"                         {
                                cmm_log_token("SEMI", yytext);
                                yylval.type_token = YYSYMBOL_SEMI;
                                return SEMI;
                            }
","                         {
                                cmm_log_token("COMMA", yytext);
                                yylval.type_token = YYSYMBOL_COMMA;
                                return COMMA;
                            }
"="                         {
                                cmm_log_token("ASSIGNOP", yytext);
                                yylval.type_token = YYSYMBOL_ASSIGNOP;
                                return ASSIGNOP;
                            }
">"|"<"|">="|"<="|"=="|"!=" {
                                cmm_log_token("RELOP", yytext);
                                yylval.type_token = YYSYMBOL_RELOP;
                                return RELOP;
                            }
"+"                         {
                                cmm_log_token("PLUS", yytext);
                                yylval.type_token = YYSYMBOL_PLUS;
                                return PLUS;
                            }
"-"                         {
                                cmm_log_token("MINUS", yytext);
                                yylval.type_token = YYSYMBOL_MINUS;
                                return MINUS;
                            }
"*"                         {
                                cmm_log_token("STAR", yytext);
                                yylval.type_token = YYSYMBOL_STAR;
                                return STAR;
                            }
"/"                         {
                                cmm_log_token("DIV", yytext);
                                yylval.type_token = YYSYMBOL_DIV;
                                return DIV;
                            }
"&&"                        {
                                cmm_log_token("AND", yytext);
                                yylval.type_token = YYSYMBOL_AND;
                                return AND;
                            }
"||"                        {
                                cmm_log_token("OR", yytext);
                                yylval.type_token = YYSYMBOL_OR;
                                return OR;
                            }
"."                         {
                                cmm_log_token("DOT", yytext);
                                yylval.type_token = YYSYMBOL_DOT;
                                return DOT;
                            }
"!"                         {
                                cmm_log_token("NOT", yytext);
                                yylval.type_token = YYSYMBOL_NOT;
                                return NOT;
                            }
"int"|"float"               { 
                                cmm_log_token("type", yytext);
                                yylval.type_type = yytext;
                                return TYPE;
                            }
"("                         {
                                cmm_log_token("LP", yytext);
                                yylval.type_token = YYSYMBOL_LP;
                                return LP;
                            }
")"                         {
                                cmm_log_token("RP", yytext);
                                yylval.type_token = YYSYMBOL_RP;
                                return RP;
                            }
"["                         {
                                cmm_log_token("LB", yytext);
                                yylval.type_token = YYSYMBOL_LB;
                                return LB;
                            }
"]"                         {
                                cmm_log_token("RB", yytext);
                                yylval.type_token = YYSYMBOL_RB;
                                return RB;
                            }
"{"                         {
                                cmm_log_token("LC", yytext);
                                yylval.type_token = YYSYMBOL_LC;
                                return LC;
                            }
"}"                         {
                                cmm_log_token("RC", yytext);
                                yylval.type_token = YYSYMBOL_RC;
                                return RC;
                            }
"struct"                    {
                                cmm_log_token("STRUCT", yytext);
                                yylval.type_token = YYSYMBOL_STRUCT;
                                return STRUCT;
                            }
"return"                    {
                                cmm_log_token("RETURN", yytext);
                                yylval.type_token = YYSYMBOL_RETURN;
                                return RETURN;
                            }
"if"                        {
                                cmm_log_token("IF", yytext);
                                yylval.type_token = YYSYMBOL_IF;
                                return IF;
                            }
"else"                      {
                                cmm_log_token("ELSE", yytext);
                                yylval.type_token = YYSYMBOL_ELSE;
                                return ELSE;
                            }
"while"                     {
                                cmm_log_token("WHILE", yytext);
                                yylval.type_token = YYSYMBOL_WHILE;
                                return WHILE;
                            }

[0-9]+\.[0-9]+              {
                                cmm_log_token("FLOAT", yytext);
                                yylval.type_float = atof(yytext);
                                return FLOAT;
                            }

[0-9]+                      {
                                cmm_log_token("INT", yytext);
                                yylval.type_int = atoi(yytext);
                                return INT;
                            }

[a-zA-Z_][a-zA-Z0-9_]*      {
                                cmm_log_token("ID", yytext);
                                yylval.type_ident = yytext;
                                return ID;
                            }

[ \t]+                      {
                                cmm_log_token("__SPACE__", yytext);
                            }
[\r\n]+                     {
                                cmm_log_token("__LINE__", "newline");
                            }

.                           { 
                                printf("\nerror: unexpected yytext = %s", yytext); 
                                printf("\ncode: %d", yytext[0]); 
                                printf("\n"); 
                            }

%%