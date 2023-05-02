#include "Method.h"
#include "ByteArray.h"
#include "Array.h"
#include "String.h"
#include "Memory.h"
#include "ByteCode.h"
#include <stdio.h>


Method* new_Method()
{
	Method* self = alloc_obj(Method);
	self->bytecode = new_ByteArray();
	self->literals = new_Array();
	return self;
}


void Method_dump(Method* self)
{
	printf("Bytecode:\n");
	size_t size = self->bytecode->size;
	int8_t* bytecode = (int8_t*) self->bytecode->array;
	int8_t src, dest;
	for (int i = 0; i < size; ++i) {
		uint8_t opcode = bytecode[i];
		switch (opcode) {
			case BC_NOP:
				printf("  NOP\n");
				break;
			case BC_LOAD_GLOBAL:
				printf("  load_global %d\n", bytecode[++i]);
				break;
			case BC_SET_LOCAL:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("  set_local %d -> %d\n", src, dest);
				break;
			case BC_TRUE:
				printf("  true -> %d\n", bytecode[++i]);
				break;
			case BC_FALSE:
				printf("  false -> %d\n", bytecode[++i]);
				break;
			case BC_NIL:
				printf("  nil -> %d\n", bytecode[++i]);
				break;
			default:
				printf("  UNKNOWN %d\n", opcode);
				break;
			}
		}

	printf("Literals:\n");
	size = self->literals->size;
	for (int i = 0; i < size; ++i) {
		// TODO: Don't assume they're all strings!
		printf("  %d: \"", -i - 1);
		String* str = (String*) self->literals->items[i];
		fwrite(str->str, str->size, 1, stdout);
		printf("\"\n");
		}
}



