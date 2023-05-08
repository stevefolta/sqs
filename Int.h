#pragma once

struct Class;
struct Object;

typedef struct Int {
	struct Class* class_;
	int value;
	} Int;
extern Int* new_Int(int value);

#define Int_value(object) (((Int*) object)->value)

extern int Int_enforce(struct Object* object, const char* name);

extern struct Class Int_class;
extern void Int_init_class();

