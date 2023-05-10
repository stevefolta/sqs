#pragma once

struct Class;
struct String;

typedef struct Object {
	struct Class* class_;
	} Object;

extern Object* Object_find_method(Object* self, struct String* name);
extern Object* Object_identity(Object* self, Object** args);


extern struct Class Object_class;
extern void Object_init_class();

