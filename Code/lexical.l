%{
    #include "predefines.h"
    #include "syntax.tab.h"
%}

%%

[0-9]+                      {
                                yylval = atoi(yytext);
                                return INT;
                            }

[0-9]+\.[0-9]+              {
                                yylval = atof(yytext);
                                return FLOAT;
                            }

[a-zA-Z_][a-zA-Z0-9_]*      {
                                yylval = yytext;
                                return ID;
                            }

";"                         { return SEMI; }
","                         { return COMMA; }
"="                         { return ASSIGNOP; }
">"|"<"|">="|"<="|"=="|"!=" { return RELOP; }
"+"                         { return PLUS; }
"-"                         { return MINUS; }
"*"                         { return STAR; }
"/"                         { return DIV; }
"&&"                        { return AND; }
"||"                        { return OR; }
"."                         { return DOT; }
"!"                         { return NOT; }
int|float                   { return TYPE; }
"("                         { return LP; }
")"                         { return RP; }
"["                         { return LB; }
"]"                         { return RB; }
"{"                         { return LC; }
"}"                         { return RC; }
struct                      { return STRUCT; }
return                      { return RETURN; }
if                          { return IF; }
else                        { return ELSE; }
while                       { return WHILE; }
[ \t]+                      ;
\n                          ;

.                           { printf("unexpected token %s\n", yytext); }

%%