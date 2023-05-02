#pragma once

// A "location", if non-negative, is an offset in the current stack frame.  If
// negative, it's the offset in the current method's literals minus one.


enum {
	BC_NOP,
	BC_LOAD_GLOBAL, 	// followed by location of global name (typically a literal).
	BC_SET_LOCAL, 	// src, dest
	BC_TRUE,	// dest
	BC_FALSE,	// dest
	BC_NIL,	// dest
	BC_BRANCH_IF_TRUE, 	// value, offset_8
	BC_BRANCH_IF_FALSE,	// value, offset_8
	BC_BRANCH, 	// offset_8
	};

