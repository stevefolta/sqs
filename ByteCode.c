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
#include "Int.h"
#include "Nil.h"
#include "Memory.h"
#include "Error.h"
#include <stdio.h>

static Object** stack = NULL;
static Object** stack_limit;
static Object** suspended_fp;
enum {
	stack_size = 1024,
	};

bool dump_requested = false;

extern Object** get_upvalue_ptr(Method* method, int local_offset, Object** frame);
extern void dump_stack(Object** frame, Object** literals, int depth);

void init_bytecode_interpreter()
{
	if (stack)
		return;

	stack = (Object**) alloc_mem(stack_size * sizeof(Object*));
	stack_limit = stack + stack_size;
	suspended_fp = stack;
}


void interpret_bytecode(struct Method* method)
{
	// Interpret.
	Object** frame = suspended_fp;
	Object** literals = method->literals->items;
	int8_t* start_pc = (int8_t*) method->bytecode->array; 	// Just for debugging.
	int8_t* pc = start_pc;
	while (true) {
		uint8_t opcode = *pc++;
		int8_t src, dest;
		ptrdiff_t offset;
		Object* value;
		uint8_t frame_adjustment;
		int args_given;
		#define DEREF(index) (index >= 0 ? frame[index] : literals[-index - 1])
		#define GET_OFFSET() { offset = ((ptrdiff_t) (int8_t) *pc++) << 8; offset |= (uint8_t) *pc++; }
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
			case BC_GET_LITERAL:
				GET_OFFSET();
				dest = *pc++;
				frame[dest] = literals[(uint16_t) offset];
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
			case BC_NOT:
				src = *pc++;
				dest = *pc++;
				frame[dest] = NOT(DEREF(src));
				break;
			case BC_BRANCH_IF_TRUE:
				src = *pc++;
				GET_OFFSET()
				value = DEREF(src);
				if (IS_TRUTHY(value))
					pc += offset;
				break;
			case BC_BRANCH_IF_FALSE:
				src = *pc++;
				GET_OFFSET()
				value = DEREF(src);
				if (!IS_TRUTHY(value))
					pc += offset;
				break;
			case BC_BRANCH_IF_NIL:
				src = *pc++;
				GET_OFFSET()
				value = DEREF(src);
				if (value == NULL)
					pc += offset;
				break;
			case BC_BRANCH_IF_NOT_NIL:
				src = *pc++;
				GET_OFFSET()
				value = DEREF(src);
				if (value)
					pc += offset;
				break;
			case BC_BRANCH:
				GET_OFFSET()
				pc += offset;
				break;

			case BC_CALL_0:
			case BC_CALL_1: case BC_CALL_2: case BC_CALL_3: case BC_CALL_4: case BC_CALL_5:
			case BC_CALL_6: case BC_CALL_7: case BC_CALL_8: case BC_CALL_9: case BC_CALL_10:
			case BC_CALL_11: case BC_CALL_12: case BC_CALL_13: case BC_CALL_14: case BC_CALL_15:
				{
				// Parameters.
				int8_t name = *pc++;
				frame_adjustment = *pc++;
				String* name_str = (String*) DEREF(name);
				args_given = opcode - BC_CALL_0;

				// Find the method.
				value = Object_find_method(frame[frame_adjustment], name_str);
				if (value == NULL) {
					Class* receiver_class = (frame[frame_adjustment] ? frame[frame_adjustment]->class_ : &Nil_class);
					fprintf(
						stderr, "Unhandled method call: \"%s\" on %s.  Stack trace:\n",
						String_c_str(name_str), String_c_str(receiver_class->name));
					dump_stack(frame, literals, 10);
					exit(EXIT_FAILURE);
					}
				}

			make_call:
				{
				// Bump the frame and save the state.
				Object** old_fp = frame;
				frame += frame_adjustment;
				frame[-3] = (Object*) old_fp;
				frame[-2] = (Object*) pc;
				frame[-1] = (Object*) literals;

				// If there weren't enough arguments, fill the rest with nil.
				int args_needed = ((Method*) value)->num_args; 	// also works for BuiltinMethod
				while (args_given < args_needed) {
					// These arg counts don't include "self".
					frame[args_given + 1] = NULL;
					args_given += 1;
					}

				// Call the method.
				if (value->class_ == &Method_class) {
					if (frame + ((Method*) value)->stack_size >= stack_limit) {
						fprintf(stderr, "Stack overflow!  Stack trace:\n");
						dump_stack(frame, literals, 10);
						exit(EXIT_FAILURE);
						}
					pc = (int8_t*) ((Method*) value)->bytecode->array;
					literals = ((Method*) value)->literals->items;
					}
				else if (value->class_ == &BuiltinMethod_class) {
					// Set "suspended_fp" to the end of our stack frame, so this can be
					// called re-entrantly.
					suspended_fp = frame + args_needed + 1;

					// Call.
					Object* result = ((BuiltinMethod*) value)->fn(frame[0], frame + 1);
					frame[-4] = result;
					goto return_from_method;
					}
				}
				break;

			case BC_FN_CALL:
				{
				// Parameters.
				int8_t fn_loc = *pc++;
				args_given = *pc++;
				frame_adjustment = *pc++;
				value = DEREF(fn_loc);
				frame[frame_adjustment] = NULL; 	// receiver is "nil"

				// Make sure it's really a function.
				if (value == NULL)
					Error("Attempt to call \"nil\" as a function.");
				if (value->class_ != &Method_class && value->class_ != &BuiltinMethod_class && value->class_ != &Class_class)
					Error("Attempt to call a non-function (a %s).", String_c_str(value->class_->name));

				// Turn calling a class into object instantiation.
				if (value->class_ == &Class_class) {
					// Create the object.
					frame[frame_adjustment] = Class_instantiate((Class*) value);

					// Turn this into an "init()" call.
					String init_str;
					String_init_static_c(&init_str, "init");
					value = Object_find_method(frame[frame_adjustment], &init_str);
					if (value == NULL) {
						// No init(), just quit, returning the new object.
						frame[frame_adjustment - 4] = frame[frame_adjustment];
						continue;
						}
					}
				}
				goto make_call;
				break;

			case BC_SUPER_CALL:
				{
				// Parameters.
				int8_t name = *pc++;
				int8_t class_loc = *pc++;
				Class* child_class = (Class*) DEREF(class_loc);
				args_given = *pc++;
				frame_adjustment = *pc++;
				String* name_str = (String*) DEREF(name);

				// Find the method.
				value = Class_find_super_method(child_class, name_str);
				if (value == NULL) {
					Class* receiver_class = (frame[frame_adjustment] ? frame[frame_adjustment]->class_ : &Nil_class);
					fprintf(
						stderr, "Unhandled method call: \"%s\" on %s.  Stack trace:\n",
						String_c_str(name_str), String_c_str(receiver_class->name));
					exit(EXIT_FAILURE);
					}
				}
				goto make_call;
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
			case BC_ARRAY_APPEND_STRINGS:
				dest = *pc++;
				src = *pc++;
				Array_append_strings((Array*) DEREF(dest), DEREF(src));
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

			case BC_GET_UPVAL:
				src = *pc++; 	// method
				value = DEREF(src);
				src = *pc++; 	// local offset
				dest = *pc++;
				frame[dest] = *get_upvalue_ptr((Method*) value, src, frame);
				break;
			case BC_SET_UPVAL:
				src = *pc++; 	// method
				value = DEREF(src);
				dest = *pc++; 	// local offset
				src = *pc++;
				*get_upvalue_ptr((Method*) value, dest, frame) = DEREF(src);
				break;

			case BC_GET_MODULE_LOCAL:
				src = *pc++; 	// module frame
				value = DEREF(src);
				src = *pc++; 	// local offset
				dest = *pc++;
				frame[dest] = ((Object**) value)[src];
				break;
			case BC_SET_MODULE_LOCAL:
				src = *pc++; 	// module frame
				value = DEREF(src);
				dest = *pc++; 	// local offset
				src = *pc++;
				((Object**) value)[dest] = DEREF(src);
				break;

			default:
				Error("Internal error: bad bytecode %d.", opcode);
				break;
			}
		}
	exit: ;
}


Object* call_method_raw(Object* method, Object* receiver, Array* arguments)
{
	static uint8_t terminator[] = { BC_TERMINATE };

	if (stack == NULL)
		init_bytecode_interpreter();

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


Object* call_object(Object* receiver, String* name, Array* arguments)
{
	// Find the method.
	Object* method = Object_find_method(receiver, name);
	if (method == NULL) {
		Class* receiver_class = (receiver ? receiver->class_ : &Nil_class);
		Error("Unhandled method call: \"%s\" on %s.", String_c_str(name), String_c_str(receiver_class->name));
		}

	return call_method_raw(method, receiver, arguments);
}


Object* call_method(struct Method* method, struct Array* arguments)
{
	return call_method_raw((Object*) method, NULL, arguments);
}


void print_object(Object* object)
{
	if (object == NULL) {
		// Shouldn't really happen...
		printf("nil");
		}
	else if (object->class_ == &String_class) {
		String* str = (String*) object;
		fwrite("\"", 1, 1, stdout);
		fwrite(str->str, str->size, 1, stdout);
		fwrite("\"", 1, 1, stdout);
		}
	else if (object->class_ == &Class_class) {
		printf("Class: ");
		String* str = ((Class*) object)->name;
		fwrite(str->str, str->size, 1, stdout);
		}
	else if (object->class_ == &Int_class)
		printf("%d", Int_value((Int*) object));
	else {
		printf("a ");
		String* str = object->class_->name;
		fwrite(str->str, str->size, 1, stdout);
		}
}

void print_loc(int loc, Array* literals)
{
	printf("[%d", loc);
	if (loc < 0) {
		printf(": ");
		print_object(literals->items[-loc - 1]);
		}
	printf("]");
}

void dump_bytecode(struct Method* method, String* class_name, String* function_name)
{
	if (function_name) {
		if (class_name)
			printf("%s.%s(%d args):\n", String_c_str(class_name), String_c_str(function_name), method->num_args);
		else
			printf("%s(%d args):\n", String_c_str(function_name), method->num_args);
		}
	else {
		printf("Args: %d\n", method->num_args);
		printf("Bytecode:\n");
		}
	size_t size = method->bytecode->size;
	int8_t* bytecode = (int8_t*) method->bytecode->array;
	int8_t src, dest;
	int16_t offset;
	#undef GET_OFFSET
	#define GET_OFFSET() { offset = ((int16_t) (int8_t) bytecode[++i]) << 8; offset |= (uint8_t) bytecode[++i]; }
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
				print_loc(src, method->literals);
				printf(" -> [%d]\n", dest);
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
			case BC_GET_LITERAL:
				GET_OFFSET();
				dest = bytecode[++i];
				printf("literal %d (", (uint16_t) offset);
				print_object(method->literals->items[(uint16_t) offset]);
				printf(") -> [%d]\n", dest);
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
			case BC_NOT:
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("not [%d] -> [%d]\n", src, dest);
				break;
			case BC_BRANCH_IF_TRUE:
				src = bytecode[++i];
				GET_OFFSET();
				printf("branch_if_true [%d], %d\n", src, (i + 1) + offset);
				break;
			case BC_BRANCH_IF_FALSE:
				src = bytecode[++i];
				GET_OFFSET()
				printf("branch_if_false [%d], %d\n", src, (i + 1) + offset);
				break;
			case BC_BRANCH_IF_NIL:
				src = bytecode[++i];
				GET_OFFSET()
				printf("branch_if_nil [%d], %d\n", src, (i + 1) + offset);
				break;
			case BC_BRANCH_IF_NOT_NIL:
				src = bytecode[++i];
				GET_OFFSET()
				printf("branch_if_not_nil [%d], %d\n", src, (i + 1) + offset);
				break;
			case BC_BRANCH:
				GET_OFFSET()
				printf("branch %d\n", (i + 1) + offset);
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
				printf("call_%d ", opcode - BC_CALL_0);
				print_loc(src, method->literals);
				printf(" stack-adjust: %d\n", (uint8_t) dest);
				break;
			case BC_FN_CALL:
			case BC_SUPER_CALL:
				{
				int8_t fn_loc = bytecode[++i];
				uint8_t num_args = bytecode[++i];
				uint8_t frame_adjustment = bytecode[++i];
				printf(opcode == BC_SUPER_CALL ? "super_call " : "fn_call ");
				print_loc(fn_loc, method->literals);
				printf("(%d args) stack-adjust: %d\n", num_args, frame_adjustment);
				}
				break;
			case BC_NEW_ARRAY:
				dest = bytecode[++i];
				printf("new_array -> [%d]\n", dest);
				break;
			case BC_ARRAY_APPEND:
				dest = bytecode[++i];
				src = bytecode[++i];
				printf("array_append ");
				print_loc(src, method->literals);
				printf(" into [%d]\n", dest);
				break;
			case BC_ARRAY_APPEND_STRINGS:
				dest = bytecode[++i];
				src = bytecode[++i];
				printf("array_append_strings ");
				print_loc(src, method->literals);
				printf(" into [%d]\n", dest);
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
				printf("dict_add (");
				print_loc(key, method->literals);
				printf(" => ");
				print_loc(src, method->literals);
				printf(") to [%d]\n", dest);
				}
				break;
			case BC_GET_UPVAL:
				{
				src = bytecode[++i];
				printf("get_upval ([%d", src);
				if (src < 0) {
					printf(": ");
					print_object(method->literals->items[-src - 1]);
					}
				src = bytecode[++i];
				dest = bytecode[++i];
				printf("], %d) -> [%d]\n", src, dest);
				}
				break;
			case BC_SET_UPVAL:
				{
				src = bytecode[++i];
				printf("set_upval (");
				print_loc(src, method->literals);
				dest = bytecode[++i];
				src = bytecode[++i];
				printf(", %d) <- [%d]\n", dest, src);
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
		if (i < -INT8_MIN)
			printf("%6d: ", -i - 1);
		else
			printf("%6d: ", i);
		print_object(method->literals->items[i]);
		printf("\n");
		}
}


Object** get_upvalue_ptr(Method* method, int local_offset, Object** frame)
{
	// Look for nearest enclosing frame for "method".
	while (frame > stack) {
		// Is the enclosing frame for "method"?
		if (frame[-1] == (Object*) method->literals->items)
			return ((Object**) frame[-3]) + local_offset;

		// Go up a frame.
		frame = (Object**) frame[-3];
		}

	Error("Internal error: upvalue reference with no enclosing frame.");
	return NULL;
}


void dump_stack(Object** frame, Object** literals, int depth)
{
	for (; depth != 0 && frame && literals; --depth) {
		const char* name = "-unknown-";
		Object* name_obj = literals[0];
		if (name_obj->class_ == &String_class)
			name = String_c_str((String*) name_obj);
		if (frame[0])
			fprintf(stderr, "\t%s on %s\n", name, String_c_str(frame[0]->class_->name));
		else
			fprintf(stderr, "\t%s\n", name);

		// Go to the next frame.
		frame = (Object**) frame[-3];
		literals = (Object**) frame[-1];
		}

	if (frame != NULL && literals != NULL)
		fprintf(stderr, "\t...\n");
}



