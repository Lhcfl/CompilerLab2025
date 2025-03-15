%{
    #include "predefines.h"
    #include "syntax.tab.h"
%}

%%
[0-9]+  {
    yylval = atoi(yytext);
    return INT;
}
"+"     { return ADD; }
"-"     { return SUB; }
"*"     { return MUL; }
"/"     { return DIV; }
\n      { return LINEBREAK; }
[ \t]+  ;
.       { printf("unexpected token %s\n", yytext); }

%%