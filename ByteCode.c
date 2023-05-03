#include "ByteCode.h"
#include "Method.h"
#include "BuiltinMethod.h"
#include "ByteArray.h"
#include "Array.h"
#include "String.h"
#include "Object.h"
#include "Boolean.h"
#include "Memory.h"
#include <stdio.h>

static Object** stack = NULL;
static Object** stack_limit;
#define stack_size 1024


void interpret_bytecode(struct Method* method)
{
	if (stack == NULL) {
		stack = (Object**) alloc_mem(stack_size * sizeof(Object*));
		stack_limit = stack + stack_size;
		}

	// Start with "self" == nil.
	stack[0] = NULL;

	// Interpret.
	Object** frame = stack;
	Object** literals = method->literals->items;
	int8_t* pc = (int8_t*) method->bytecode->array;
	while (true) {
		uint8_t opcode = *pc++;
		int8_t src, dest;
		Object* value;
		#define DEREF(index) (index >= 0 ? frame[index] : literals[-index + 1])
		#define IS_TRUTHY(obj) (obj != NULL && obj != &false_obj)
		switch (opcode) {
			case BC_NOP:
				break;
			case BC_TERMINATE:
				goto exit;
			case BC_SET_LOCAL:
				src = *pc++;
				dest = *pc++;
				frame[dest] = (dest >= 0 ? frame[src] : literals[-src]);
				break;
			case BC_TRUE:
				dest = *pc++;
				frame[dest] = &true_obj;
				break;
			case BC_FALSE:
				dest = *pc++;
				frame[dest] = &false_obj;
				break;
			case BC_NIL:
				dest = *pc++;
				frame[dest] = NULL;
				break;
			case BC_LOAD_GLOBAL:
				//*** TODO
				break;
			case BC_BRANCH_IF_TRUE:
				src = *pc++;
				dest = *pc++;
				value = DEREF(src);
				if (IS_TRUTHY(value))
					pc += dest;
				break;
			case BC_BRANCH_IF_FALSE:
				src = *pc++;
				dest = *pc++;
				value = DEREF(src);
				if (!IS_TRUTHY(value))
					pc += dest;
				break;
			case BC_BRANCH:
				dest = *pc++;
				pc += dest;
				break;

			case BC_CALL_0:
			case BC_CALL_1: case BC_CALL_2: case BC_CALL_3: case BC_CALL_4: case BC_CALL_5:
			case BC_CALL_6: case BC_CALL_7: case BC_CALL_8: case BC_CALL_9: case BC_CALL_10:
			case BC_CALL_11: case BC_CALL_12: case BC_CALL_13: case BC_CALL_14: case BC_CALL_15:
				{
				// Parameters.
				int8_t name = *pc++;
				uint8_t frame_adjustment = *pc++;

				// Bump the frame and save the state.
				Object** old_fp = frame;
				frame += frame_adjustment;
				frame[-3] = (Object*) old_fp;
				frame[-2] = (Object*) pc;
				frame[-1] = (Object*) literals;

				// Find the method.
				Object* method = Object_find_method(frame[0], (String*) DEREF(name));

				// If there weren't enough arguments, fill the rest with nil.
				int args_needed = ((Method*) method)->num_args; 	// also works for BuiltinMethod
				int args_given = opcode - BC_CALL_0;
				while (args_given < args_needed) {
					// These arg counts don't include "self".
					frame[args_given + 1] = NULL;
					args_given += 1;
					}

				// Call the method.
				if (method->class_ == &Method_class) {
					pc = (int8_t*) ((Method*) method)->bytecode->array;
					literals = ((Method*) method)->literals->items;
					}
				else if (method->class_ == &BuiltinMethod_class) {
					Object* result = ((BuiltinMethod*) method)->fn(frame[0], frame + 1);
					frame[-4] = result;
					goto return_from_method;
					}
				}
				break;

			case BC_RETURN_NIL:
				frame[-4] = NULL;
				goto return_from_method;
			case BC_RETURN:
				src = *pc++;
				frame[-4] = DEREF(src);
				// vv fall through vv
			return_from_method:
				pc = (int8_t*) frame[-2];
				literals = (Object**) frame[-1];
				frame = (Object**) frame[-3];
				break;
			}
		}
	exit: ;
}


void dump_bytecode(struct Method* method)
{
	printf("Args: %d\n", method->num_args);
	printf("Bytecode:\n");
	size_t size = method->bytecode->size;
	int8_t* bytecode = (int8_t*) method->bytecode->array;
	int8_t src, dest;
	for (int i = 0; i < size; ++i) {
		uint8_t opcode = bytecode[i];
		printf("%6d:  ", i);
		switch (opcode) {
			case BC_NOP:
				printf("NOP\n");
				break;
			case BC_SET_LOCAL:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("[%d] -> [%d]\n", src, dest);
				break;
			case BC_TRUE:
				printf("true -> [%d]\n", bytecode[++i]);
				break;
			case BC_FALSE:
				printf("false -> [%d]\n", bytecode[++i]);
				break;
			case BC_NIL:
				printf("nil -> [%d]\n", bytecode[++i]);
				break;
			case BC_LOAD_GLOBAL:
				printf("load_global %d\n", bytecode[++i]);
				break;
			case BC_BRANCH_IF_TRUE:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("branch_if_true [%d], %d\n", src, i + dest + 1);
				break;
			case BC_BRANCH_IF_FALSE:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("branch_if_false [%d], %d\n", src, i + dest + 1);
				break;
			case BC_BRANCH:
				dest = bytecode[++i];
				printf("branch %d\n", i + dest + 1);
				break;
			case BC_RETURN:
				src = bytecode[++i];
				printf("return [%d]\n", src);
				break;
			case BC_RETURN_NIL:
				printf("return nil\n");
				break;
			case BC_TERMINATE:
				printf("terminate\n");
				break;
			case BC_CALL_0:
			case BC_CALL_1: case BC_CALL_2: case BC_CALL_3: case BC_CALL_4: case BC_CALL_5:
			case BC_CALL_6: case BC_CALL_7: case BC_CALL_8: case BC_CALL_9: case BC_CALL_10:
			case BC_CALL_11: case BC_CALL_12: case BC_CALL_13: case BC_CALL_14: case BC_CALL_15:
				printf("call_%d\n", opcode - BC_CALL_0);
				break;
			default:
				printf("UNKNOWN %d\n", opcode);
				break;
			}
		}

	printf("Literals:\n");
	size = method->literals->size;
	for (int i = 0; i < size; ++i) {
		// TODO: Don't assume they're all strings!
		printf("%6d: \"", -i - 1);
		String* str = (String*) method->literals->items[i];
		fwrite(str->str, str->size, 1, stdout);
		printf("\"\n");
		}
}


