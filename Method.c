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
	Class_init_static(&Method_class, "Method", 3);
}


Method* new_Method()
{
	Method* self = alloc_obj(Method);
	self->class_ = &Method_class;
	self->bytecode = new_ByteArray();
	self->literals = new_Array();
	return self;
}


void Method_dump(Method* self)
{
	printf("Args: %d\n", self->num_args);
	printf("Bytecode:\n");
	size_t size = self->bytecode->size;
	int8_t* bytecode = (int8_t*) self->bytecode->array;
	int8_t src, dest;
	for (int i = 0; i < size; ++i) {
		uint8_t opcode = bytecode[i];
		printf("%6d:  ", i);
		switch (opcode) {
			case BC_NOP:
				printf("NOP\n");
				break;
			case BC_LOAD_GLOBAL:
				printf("load_global %d\n", bytecode[++i]);
				break;
			case BC_SET_LOCAL:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("set_local %d -> %d\n", src, dest);
				break;
			case BC_TRUE:
				printf("true -> %d\n", bytecode[++i]);
				break;
			case BC_FALSE:
				printf("false -> %d\n", bytecode[++i]);
				break;
			case BC_NIL:
				printf("nil -> %d\n", bytecode[++i]);
				break;
			case BC_BRANCH_IF_TRUE:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("branch_if_true %d, %d\n", src, i + dest + 1);
				break;
			case BC_BRANCH_IF_FALSE:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("branch_if_false %d, %d\n", src, i + dest + 1);
				break;
			case BC_BRANCH:
				dest = bytecode[++i];
				printf("branch %d\n", i + dest + 1);
				break;
			default:
				printf("UNKNOWN %d\n", opcode);
				break;
			}
		}

	printf("Literals:\n");
	size = self->literals->size;
	for (int i = 0; i < size; ++i) {
		// TODO: Don't assume they're all strings!
		printf("%6d: \"", -i - 1);
		String* str = (String*) self->literals->items[i];
		fwrite(str->str, str->size, 1, stdout);
		printf("\"\n");
		}
}



