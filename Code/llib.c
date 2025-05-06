#include "llib.h"
#include <stdlib.h>
#include <string.h>

LString MakeLString() {
    LString ret   = malloc(sizeof(struct LStringBody));
    ret->str      = malloc(sizeof(char) * DEFAULT_LARRY_CAPACITY);
    ret->capacity = DEFAULT_LARRY_CAPACITY;
    ret->size     = 0;
    return ret;
}

LString MakeLStringWith(const char* init) {
    LString ret     = malloc(sizeof(struct LStringBody));
    size_t  initlen = strlen(init);
    size_t  capa =
        initlen < DEFAULT_LARRY_CAPACITY ? DEFAULT_LARRY_CAPACITY : initlen;

    ret->str      = malloc(sizeof(char) * capa);
    ret->capacity = capa;
    ret->size     = initlen;
    strcpy(ret->str, init);
    return ret;
}

void LStringAppend(LString str, LString app) {
    if (str->size + app->size >= str->capacity) {
        str->capacity = (str->size + app->size) * 2;
        str->str      = realloc(str->str, sizeof(char) * str->capacity);
    }

    strcpy(str->str + str->size, app->str);
    str->size += app->size;
}

void LStringPush(LString str, const char* app) {
    size_t app_len = strlen(app);
    if (str->size + app_len >= str->capacity) {
        str->capacity = (str->size + app_len) * 2;
        str->str      = realloc(str->str, sizeof(char) * str->capacity);
    }

    strcpy(str->str + str->size, app);
    str->size += app_len;
}

void LStringFree(LString str) {
    free(str->str);
    free(str);
}

LString LStringClone(LString from) {
    LString ret   = malloc(sizeof(struct LStringBody));
    ret->str      = malloc(sizeof(char) * from->capacity);
    ret->size     = from->size;
    ret->capacity = from->capacity;
    strcpy(ret->str, from->str);
    return ret;
}