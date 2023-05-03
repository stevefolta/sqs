#pragma once

// A "location", if non-negative, is an offset in the current stack frame.  If
// negative, it's the offset in the current method's literals minus one.


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
	BC_CALL_0 = 16,
	BC_CALL_1, BC_CALL_2, BC_CALL_3, BC_CALL_4, BC_CALL_5,
	BC_CALL_6, BC_CALL_7, BC_CALL_8, BC_CALL_9, BC_CALL_10, 
	BC_CALL_11, BC_CALL_12, BC_CALL_13, BC_CALL_14, BC_CALL_15, 
	};


struct Method;

extern void interpret_bytecode(struct Method* method);
extern void dump_bytecode(struct Method* method);

