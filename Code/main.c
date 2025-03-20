#include "predefines.h"
#include "globals.h"
#include "globals.cpp"
#include "syntax.tab.h"
#include <stdio.h>

int main() {
    printf("\n=====================\n\n");
    yyparse();
    printf("\n\n======================\n\n");
    cmm_log_node(&cmm_parsed_root);
}