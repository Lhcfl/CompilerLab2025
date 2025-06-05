#pragma once
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_LARRY_CAPACITY 15ull
#define DefineLArray(Type)                                                    \
    typedef struct LArrayOf##Type {                                           \
        Type*  data;                                                          \
        size_t size;                                                          \
        size_t capacity;                                                      \
    } LArrayOf##Type;                                                         \
    static inline LArrayOf##Type* NewLArrayOf##Type() {                       \
        LArrayOf##Type* arr = malloc(sizeof(LArrayOf##Type));                 \
        arr->data     = (Type*)malloc(sizeof(Type) * DEFAULT_LARRY_CAPACITY); \
        arr->size     = 0;                                                    \
        arr->capacity = DEFAULT_LARRY_CAPACITY;                               \
        return arr;                                                           \
    }                                                                         \
    static inline void FreeLArrayOf##Type(LArrayOf##Type* arr) {              \
        free(arr->data);                                                      \
    }                                                                         \
    static inline void PushLArrayOf##Type(LArrayOf##Type* arr, Type val) {    \
        if (arr->size >= arr->capacity) {                                     \
            arr->capacity *= 2;                                               \
            arr->data =                                                       \
                (Type*)realloc(arr->data, sizeof(Type) * arr->capacity);      \
        }                                                                     \
        arr->data[arr->size++] = val;                                         \
    }

typedef struct LStringBody {
    char*  str;
    size_t size;
    size_t capacity;
} LStringBody;

typedef LStringBody* LString;

LString MakeLString();
LString MakeLStringWith(const char* init);
void    LStringAppend(LString str, LString app);
void    LStringPush(LString str, const char* app);
void    LStringFree(LString str);
LString LStringClone(LString from);

DefineLArray(LString)