#pragma once

#include "Class.h"
#include "Object.h"
#include <stdbool.h>

extern Object true_obj;
extern Object false_obj;


inline Object* make_bool(bool value)
{
	return (value ? &true_obj : &false_obj);
}

