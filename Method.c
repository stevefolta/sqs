#include "Method.h"
#include "ByteArray.h"
#include "Array.h"
#include "Memory.h"


Method* new_Method()
{
	Method* self = alloc_obj(Method);
	self->bytecode = new_ByteArray();
	self->literals = new_Array();
	return self;
}



