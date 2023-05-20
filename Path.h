#pragma once

struct Class;
struct String;

typedef struct Path {
	struct Class* class_;
	char* path;
	} Path;

extern struct Class Path_class;
extern void Path_init_class();


