#ifndef LINCA_BYYL_PREDEFINES
#define LINCA_BYYL_PREDEFINES

#include <stdio.h>

extern int yylex(void);
extern int fileno(FILE*);

void yyerror(char* msg) { fprintf(stderr, "error: %s\n", msg); }

void cmm_log_token(char* ty, char* yytext) { printf("[Token { name: \"%s\", val: \"%s\" }] ", ty, yytext); }

#endif