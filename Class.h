#pragma once

struct String;
struct Dict;

typedef struct Class {
	struct Class* class_;
	struct String* name;
	struct Class* superclass;
	int total_num_slots;
	struct Dict* methods;
	} Class;

extern void Class_init_static(Class* self, const char* name, int total_num_slots);

extern void Class_init_class();


