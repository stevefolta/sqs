#pragma once

struct Class;
struct String;

typedef struct Path {
	struct Class* class_;
	char* path;
		// "path" is stored as a null-terminated C string.
	} Path;

extern struct Class Path_class;
extern void Path_init_class();


