#include "syntax.tab.h"
#include <stdio.h>

int main() {
    printf("done;\n");
    yyparse();
}