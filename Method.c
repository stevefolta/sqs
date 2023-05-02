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
	uint8_t* bytecode = self->bytecode->array;
	for (int i = 0; i < size; ++i) {
		uint8_t opcode = bytecode[i];
		switch (opcode) {
			case BC_NOP:
				printf("  NOP\n");
				break;
			case BC_LOAD_GLOBAL:
				printf("  load_global %d\n", bytecode[++i]);
				break;
			}
		}

	printf("Literals:\n");
	size = self->literals->size;
	for (int i = 0; i < size; ++i) {
		// TODO: Don't assume they're all strings!
		printf("%d: \"", -i - 1);
		String* str = (String*) self->literals->items[i];
		fwrite(str->str, str->size, 1, stdout);
		printf("\"\n");
		}
}



