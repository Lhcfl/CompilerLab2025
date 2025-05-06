#pragma once

#include "predefines.h"

char*         cmm_ty_make_array_typename(CMM_SEM_TYPE ty);
char*         cmm_ty_make_fn_typename(CMM_SEM_TYPE ty);
CMM_SEM_TYPE  cmm_ty_make_primitive(char* name);
CMM_SEM_TYPE  cmm_ty_make_array(CMM_SEM_TYPE* inner, int size);
CMM_SEM_TYPE  cmm_ty_make_func(CMM_SEM_TYPE* inner, int size);
CMM_SEM_TYPE  cmm_ty_make_struct(char* name, CMM_SEM_TYPE* inner, int size);
CMM_SEM_TYPE  cmm_ty_make_error();
int           cmm_ty_eq(CMM_SEM_TYPE t1, CMM_SEM_TYPE t2);
int           cmm_ty_fitable(CMM_SEM_TYPE t1, CMM_SEM_TYPE t2);
CMM_SEM_TYPE* cmm_ty_field_of_struct(CMM_SEM_TYPE prod, char* field);
CMM_SEM_TYPE  cmm_create_function_type(int size, ...);
int           cmm_offset_of_struct_field(CMM_SEM_TYPE prod, char* field);
void          cmm_free_ty(CMM_SEM_TYPE ty);