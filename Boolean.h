#pragma once

#include "Class.h"
#include "Object.h"
#include <stdbool.h>

extern Object true_obj;
extern Object false_obj;


#define make_bool(value) ((value) ? &true_obj : &false_obj)
#define IS_TRUTHY(obj) ((obj) != NULL && (obj) != &false_obj)
#define NOT(obj) (IS_TRUTHY(obj) ? &false_obj : &true_obj)

extern void Boolean_init_class();


