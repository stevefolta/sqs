#include "ByteCode.h"
#include "Method.h"
#include "BuiltinMethod.h"
#include "ByteArray.h"
#include "Array.h"
#include "String.h"
#include "Object.h"
#include "Boolean.h"
#include "Dict.h"
#include "Class.h"
#include "Nil.h"
#include "Memory.h"
#include "Error.h"
#include <stdio.h>

static Object** stack = NULL;
static Object** stack_limit;
static Object** suspended_fp;
#define stack_size 1024


void init_bytecode_interpreter()
{
	if (stack)
		return;

	stack = (Object**) alloc_mem(stack_size * sizeof(Object*));
	stack_limit = stack + stack_size;
	suspended_fp = stack;

	// Start with "self" == nil.
	// This is for the initial "script" method, which doesn't have a receiver or
	// arguments.
	stack[0] = NULL;
}


void interpret_bytecode(struct Method* method)
{
	if (stack == NULL)
		init_bytecode_interpreter();

	// Interpret.
	Object** frame = suspended_fp;
	Object** literals = method->literals->items;
	int8_t* pc = (int8_t*) method->bytecode->array;
	while (true) {
		uint8_t opcode = *pc++;
		int8_t src, dest;
		Object* value;
		#define DEREF(index) (index >= 0 ? frame[index] : literals[-index - 1])
		#define IS_TRUTHY(obj) (obj != NULL && obj != &false_obj)
		switch (opcode) {
			case BC_NOP:
				break;
			case BC_TERMINATE:
				goto exit;
			case BC_SET_LOCAL:
				src = *pc++;
				dest = *pc++;
				frame[dest] = DEREF(src);
				break;
			case BC_GET_IVAR:
				src = *pc++;
				dest = *pc++;
				frame[dest] = ((Object**) frame[0])[src + 1];
				break;
			case BC_SET_IVAR:
				dest = *pc++;
				src = *pc++;
				((Object**) frame[0])[dest + 1] = DEREF(src);
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
			case BC_BRANCH_IF_NIL:
				src = *pc++;
				dest = *pc++;
				value = DEREF(src);
				if (value == NULL)
					pc += dest;
				break;
			case BC_BRANCH_IF_NOT_NIL:
				src = *pc++;
				dest = *pc++;
				value = DEREF(src);
				if (value)
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
				String* name_str = (String*) DEREF(name);
				Object* method = Object_find_method(frame[0], name_str);
				if (method == NULL) {
					Class* receiver_class = (frame[0] ? frame[0]->class_ : &Nil_class);
					Error("Unhandled method call: \"%s\" on %s.", String_c_str(name_str), String_c_str(receiver_class->name));
					}

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
					// Set "suspended_fp" to the end of our stack frame, so this can be
					// called re-entrantly.
					suspended_fp = frame + args_needed + 1;

					// Call.
					Object* result = ((BuiltinMethod*) method)->fn(frame[0], frame + 1);
					frame[-4] = result;
					goto return_from_method;
					}
				}
				break;

			case BC_FN_CALL:
				{
				// Parameters.
				int8_t fn_loc = *pc++;
				uint8_t args_given = *pc++;
				uint8_t frame_adjustment = *pc++;

				// Bump the frame and save the state.
				Object** old_fp = frame;
				frame += frame_adjustment;
				frame[-3] = (Object*) old_fp;
				frame[-2] = (Object*) pc;
				frame[-1] = (Object*) literals;

				// Make sure it's really a function.
				Object* method = DEREF(fn_loc);
				if (method == NULL || (method->class_ != &Method_class && method->class_ != &BuiltinMethod_class && method->class_ != &Class_class))
					Error("Attempt to call a non-function.");

				// Turn calling a class into object instantiation.
				if (method->class_ == &Class_class) {
					// Create the object.
					frame[0] = Class_instantiate((Class*) method);

					// Turn this into an "init()" call.
					String init_str;
					String_init_static_c(&init_str, "init");
					method = Object_find_method(frame[0], &init_str);
					if (method == NULL) {
						// No init(), just quit.
						frame[-4] = frame[0];
						frame -= frame_adjustment;
						continue;
						}
					}

				// If there weren't enough arguments, fill the rest with nil.
				int args_needed = ((Method*) method)->num_args; 	// also works for BuiltinMethod
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
					// Set "suspended_fp" to the end of our stack frame, so this can be
					// called re-entrantly.
					suspended_fp = frame + args_needed + 1;

					// Call.
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

			case BC_NEW_ARRAY:
				dest = *pc++;
				frame[dest] = (Object*) new_Array();
				break;
			case BC_ARRAY_APPEND:
				dest = *pc++;
				src = *pc++;
				Array_append((Array*) DEREF(dest), DEREF(src));
				break;
			case BC_ARRAY_JOIN:
				src = *pc++;
				dest = *pc++;
				frame[dest] = (Object*) Array_join((Array*) DEREF(src), NULL);
				break;

			case BC_NEW_DICT:
				dest = *pc++;
				frame[dest] = (Object*) new_Dict();
				break;
			case BC_DICT_ADD:
				dest = *pc++;
				src = *pc++; 	// key
				value = DEREF(src);
				src = *pc++; 	// value
				Dict_set_at((Dict*) DEREF(dest), (String*) value, DEREF(src));
				break;
			}
		}
	exit: ;
}


Object* call_method(Object* receiver, String* name, Array* arguments)
{
	static uint8_t terminator[] = { BC_TERMINATE };

	// Find the method.
	Object* method = Object_find_method(receiver, name);
	if (method == NULL) {
		Class* receiver_class = (receiver ? receiver->class_ : &Nil_class);
		Error("Unhandled method call: \"%s\" on %s.", String_c_str(name), String_c_str(receiver_class->name));
		}

	// If it's a BuiltinMethod, we can just call it.
	if (method->class_ == &BuiltinMethod_class)
		return ((BuiltinMethod*) method)->fn(receiver, (arguments ? arguments->items: NULL));
	else if (method->class_ != &Method_class)
		Error("Internal error: attempt to call a non-method.");

	// Set up the stack frame for the call.
	Object** orig_fp = suspended_fp;
	suspended_fp += frame_saved_area_size;
	Object** frame = suspended_fp;
	frame[-3] = (Object*) orig_fp;
	frame[-2] = (Object*) terminator;
	frame[-1] = NULL; 	// No literals in the terminator.
	frame[0] = receiver;

	// Copy the arguments to the frame.
	int args_given = 0;
	if (arguments) {
		args_given = arguments->size;
		for (int i = 0; i < args_given; ++i)
			frame[i + 1] = arguments->items[i];
		}

	// If there weren't enough arguments, fill the rest with nil.
	int args_needed = ((Method*) method)->num_args; 	// also works for BuiltinMethod
	while (args_given < args_needed) {
		// These arg counts don't include "self".
		frame[args_given + 1] = NULL;
		args_given += 1;
		}

	// Call the method.
	interpret_bytecode((Method*) method);
	Object* result = frame[-4];

	// Clean up and return.
	suspended_fp = orig_fp;
	return result;
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
			case BC_GET_IVAR:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("get_ivar %d -> [%d]\n", src, dest);
				break;
			case BC_SET_IVAR:
				dest = bytecode[++i];
				src = bytecode[++i];
				printf("set_ivar %d <- [%d]\n", dest, src);
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
			case BC_BRANCH_IF_NIL:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("branch_if_nil [%d], %d\n", src, i + dest + 1);
				break;
			case BC_BRANCH_IF_NOT_NIL:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("branch_if_not_nil [%d], %d\n", src, i + dest + 1);
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
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("call_%d [%d] stack-adjust: %d\n", opcode - BC_CALL_0, src, (uint8_t) dest);
				break;
			case BC_FN_CALL:
				{
				int8_t fn_loc = bytecode[++i];
				uint8_t num_args = bytecode[++i];
				uint8_t frame_adjustment = bytecode[++i];
				printf("fn_call [%d](%d args) stack-adjust: %d\n", fn_loc, num_args, frame_adjustment);
				}
				break;
			case BC_NEW_ARRAY:
				dest = bytecode[++i];
				printf("new_array -> [%d]\n", dest);
				break;
			case BC_ARRAY_APPEND:
				dest = bytecode[++i];
				src = bytecode[++i];
				printf("array_append [%d] into [%d]\n", src, dest);
				break;
			case BC_ARRAY_JOIN:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("array_join [%d] -> [%d]\n", src, dest);
				break;
			case BC_NEW_DICT:
				dest = bytecode[++i];
				printf("new_dict -> [%d]\n", dest);
				break;
			case BC_DICT_ADD:
				{
				dest = bytecode[++i];
				int8_t key = bytecode[++i];
				src = bytecode[++i];
				printf("dict_add ([%d] => [%d]) to [%d]\n", key, src, dest);
				}
				break;
			default:
				printf("UNKNOWN %d\n", opcode);
				break;
			}
		}

	printf("Literals:\n");
	size = method->literals->size;
	for (int i = 0; i < size; ++i) {
		printf("%6d: ", -i - 1);
		String* str = (String*) method->literals->items[i];
		if (str->class_ == &String_class) {
			fwrite("\"", 1, 1, stdout);
			fwrite(str->str, str->size, 1, stdout);
			fwrite("\"", 1, 1, stdout);
			}
		else {
			printf("a ");
			str = str->class_->name;
			fwrite(str->str, str->size, 1, stdout);
			}
		printf("\n");
		}
}



