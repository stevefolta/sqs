#pragma once

// A "location", if non-negative, is an offset in the current stack frame.  If
// negative, it's the offset in the current method's literals minus one.

struct Object;
struct Array;
struct String;

enum {
	BC_NOP,
	BC_SET_LOCAL, 	// src, dest
	BC_TRUE,	// dest
	BC_FALSE,	// dest
	BC_NIL,	// dest
	BC_LOAD_GLOBAL, 	// followed by location of global name (typically a literal).
	BC_BRANCH_IF_TRUE, 	// value, offset_8
	BC_BRANCH_IF_FALSE,	// value, offset_8
	BC_BRANCH, 	// offset_8
	BC_RETURN, 	// value
	BC_RETURN_NIL,
	BC_TERMINATE,

	// Method calls.  The low 4 bits specify the number of arguments.
	// Followed by value for the method name.
	// Followed by the "frame adjustment".
	BC_CALL_0 = 16,
	BC_CALL_1, BC_CALL_2, BC_CALL_3, BC_CALL_4, BC_CALL_5,
	BC_CALL_6, BC_CALL_7, BC_CALL_8, BC_CALL_9, BC_CALL_10, 
	BC_CALL_11, BC_CALL_12, BC_CALL_13, BC_CALL_14, BC_CALL_15, 

	BC_FN_CALL,
	// Followed by location of the function.
	// Followed by number of arguments.
	// Followed by the "frame adjustment".

	BC_NEW_ARRAY, 	// dest
	BC_ARRAY_APPEND,	// array, item
	BC_ARRAY_JOIN,	// array, dest
	};

/* A call frame on the stack looks like this:
	frame[-4]: where the return value goes
	frame[-3]: saved frame pointer
	frame[-2]: saved instruction pointer
	frame[-1]: saved literals
	frame[0]: self
	frame[1]: first argument
	...
	frame[n]: local variables and temporaries
*/
#define frame_saved_area_size 4


struct Method;

extern void interpret_bytecode(struct Method* method);
extern struct Object* call_method(struct Object* receiver, struct String* name, struct Array* arguments);
extern void dump_bytecode(struct Method* method);

