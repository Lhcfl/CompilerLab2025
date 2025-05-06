#include "type_system.h"
#include "predefines.h"
#include <stdlib.h>
#include <string.h>

/// @returns inner[size]
// 同C语言一样，只要数组的基类型和维
// 数相同我们即认为类型是匹配的，例如int a[10][2]和int b[5][3]即属于同一类型；
// 所以只需要记住维数
char* cmm_ty_make_array_typename(CMM_SEM_TYPE ty) {
    // char* buffer = malloc(sizeof(char) * (strlen(ty.inner->name) + 20));
    // sprintf(buffer, "%s[%d]", ty.inner->name, ty.size);
    return cmm_concat_string(2, ty.inner->name, "[]");
}

/// @returns arg -> arg -> arg -> ... -> ret
char* cmm_ty_make_fn_typename(CMM_SEM_TYPE ty) {
    size_t len = 8;
    for (int i = 0; i < ty.size; i++) {
        len += strlen(ty.inner[i].name);
        len += 4;   // 4 for " -> "
    }
    char* ret = malloc(sizeof(char) * (len + 3));
    ret[0]    = 0;
    if (ty.size == 1) { strcpy(ret, "() -> "); }
    int offset = strlen(ret);
    for (int i = 0; i < ty.size; i++) {
        strcpy(ret + offset, ty.inner[i].name);
        offset += strlen(ty.inner[i].name);
        if (i == ty.size - 1) { break; }
        strcpy(ret + offset, " -> ");
        offset += 4;
    }
    return ret;
}

CMM_SEM_TYPE cmm_ty_make_primitive(char* name) {
    CMM_SEM_TYPE ret;
    ret.kind   = CMM_PRIMITIVE_TYPE;
    ret.name   = name;
    ret.bind   = NULL;
    ret.inner  = NULL;
    ret.size   = 0;
    ret.bytes4 = 1;
    return ret;
}

CMM_SEM_TYPE cmm_ty_make_array(CMM_SEM_TYPE* inner, int size) {
    CMM_SEM_TYPE ret;
    ret.kind   = CMM_ARRAY_TYPE;
    ret.size   = size;
    ret.inner  = inner;
    ret.bind   = NULL;
    ret.bytes4 = size * inner->bytes4;
    ret.name   = cmm_ty_make_array_typename(ret);
    return ret;
}

CMM_SEM_TYPE cmm_ty_make_struct(char* name, CMM_SEM_TYPE* inner, int size) {
    CMM_SEM_TYPE ret;
    ret.kind  = CMM_PROD_TYPE;
    ret.size  = size;
    ret.inner = inner;
    ret.bind  = NULL;
    ret.name  = name;

    ret.bytes4 = 0;
    for (int i = 0; i < size; i++) { ret.bytes4 += inner[i].bytes4; }
    return ret;
}

int cmm_offset_of_struct_field(CMM_SEM_TYPE prod, char* field) {
    int offset = 0;
    for (int i = 0; i < prod.size; i++) {
        if (strcmp(prod.inner[i].bind, field) == 0) { return offset; }
        offset += prod.inner[i].bytes4;
    }
    return 0;
}

CMM_SEM_TYPE cmm_ty_make_func(CMM_SEM_TYPE* inner, int size) {
    CMM_SEM_TYPE ret;
    ret.kind   = CMM_FUNCTION_TYPE;
    ret.size   = size;
    ret.inner  = inner;
    ret.bind   = NULL;
    ret.name   = cmm_ty_make_fn_typename(ret);
    ret.bytes4 = 1;
    return ret;
}

/// usage: cmm_create_function_type(2, "int", "float"); => int -> float
/// usage: cmm_create_function_type(1, "int"); => () -> float
CMM_SEM_TYPE cmm_create_function_type(int size, ...) {
    va_list args;
    va_start(args, size);

    CMM_SEM_TYPE* inner = malloc(sizeof(CMM_SEM_TYPE) * size);

    for (int i = 0; i < size; i++) {
        char* typename = va_arg(args, char*);
        inner[i]       = cmm_ty_make_primitive(typename);
    }
    va_end(args);

    return cmm_ty_make_func(inner, size);
}

CMM_SEM_TYPE cmm_ty_make_error() {
    CMM_SEM_TYPE ret;
    ret.kind   = CMM_ERROR_TYPE;
    ret.size   = 0;
    ret.inner  = NULL;
    ret.bind   = NULL;
    ret.name   = "(error)";
    ret.bytes4 = 0;
    return ret;
}

/// @returns 0 如果不同
/// @returns 1 如果相同
int cmm_ty_eq(CMM_SEM_TYPE t1, CMM_SEM_TYPE t2) {
    return strcmp(t1.name, t2.name) == 0;
}

int cmm_ty_fitable(CMM_SEM_TYPE t1, CMM_SEM_TYPE t2) {
    /// 我们认为错误类型能匹配任意类型
    if (t1.kind == CMM_ERROR_TYPE) { return 1; }
    if (t2.kind == CMM_ERROR_TYPE) { return 1; }
    /// 否则直接匹配类型名
    return strcmp(t1.name, t2.name) == 0;
}

/// @returns typeof(prod.field)
CMM_SEM_TYPE* cmm_ty_field_of_struct(CMM_SEM_TYPE prod, char* field) {
    for (int i = 0; i < prod.size; i++) {
        if (strcmp(prod.inner[i].bind, field) == 0) { return &(prod.inner[i]); }
    }
    return NULL;
}

void cmm_free_ty(CMM_SEM_TYPE ty) {
    free(ty.bind);
    switch (ty.kind) {
        case CMM_PRIMITIVE_TYPE: break;
        case CMM_ERROR_TYPE: break;
        case CMM_ARRAY_TYPE:
            cmm_free_ty(ty.inner[0]);
            free(ty.inner);
            break;
        case CMM_PROD_TYPE:
        case CMM_FUNCTION_TYPE:
            for (int i = 0; i < ty.size; i++) { cmm_free_ty(ty.inner[i]); }
            free(ty.inner);
            free(ty.name);
            break;
    }
}