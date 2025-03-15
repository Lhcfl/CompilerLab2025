#ifndef LINCA_BYYL_PREDEFINES
#define LINCA_BYYL_PREDEFINES

#include <stdio.h>

void yyerror(char* msg) { fprintf(stderr, "error: %s\n", msg); }

#endif