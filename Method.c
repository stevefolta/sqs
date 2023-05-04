#include "Method.h"
#include "ByteArray.h"
#include "Array.h"
#include "String.h"
#include "Memory.h"
#include "ByteCode.h"
#include "Class.h"
#include <stdio.h>

struct Class Method_class;

void Method_init_class()
{
	init_static_class(Method);
}


Method* new_Method(int num_args)
{
	Method* self = alloc_obj(Method);
	self->class_ = &Method_class;
	self->bytecode = new_ByteArray();
	self->literals = new_Array();
	self->num_args = num_args;
	return self;
}


void Method_dump(Method* self)
{
	dump_bytecode(self);
}



