#pragma once

// A "location", if non-negative, is an offset in the current stack frame.  If
// negative, it's the offset in the current method's literals minus one.


enum {
	BC_NOP,
	BC_LOAD_GLOBAL, 	// followed by location of global name (typically a global).
	};

