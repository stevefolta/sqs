#pragma once

struct Class;

typedef struct Object {
	struct Class* class_;
	} Object;


extern struct Class Object_class;
extern void Object_init_class();

