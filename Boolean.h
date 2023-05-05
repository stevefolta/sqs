#pragma once

#include "Class.h"
#include "Object.h"
#include <stdbool.h>

extern Object true_obj;
extern Object false_obj;


#define make_bool(value) (value ? &true_obj : &false_obj)

extern void Boolean_init_class();


