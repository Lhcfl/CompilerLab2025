#include "syntax.tab.h"
#include <stdio.h>

int main() {
    printf("start;\n=====================\n\n");
    yyparse();
    printf("\n\n======================\nend;\n");
}