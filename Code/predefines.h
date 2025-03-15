#ifndef LINCA_BYYL_PREDEFINES
#define LINCA_BYYL_PREDEFINES

#include <stdio.h>

extern int yylex(void);
extern int fileno(FILE*);

void yyerror(char* msg) { fprintf(stderr, "error: %s\n", msg); }

#endif