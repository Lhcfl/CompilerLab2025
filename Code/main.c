#include "syntax.tab.h"
#include <stdio.h>

int main() {
    printf("start;\n");
    yyparse();
    printf("end;\n");
}