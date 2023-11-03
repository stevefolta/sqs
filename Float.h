#pragma once

struct Class;
struct Object;

typedef struct Float {
	struct Class* class_;
	double value;
	} Float;
extern Float* new_Float(double value);

#define Float_value(object) (((Float*) object)->value)

extern double Float_enforce(struct Object* object, const char* name);

extern struct Class Float_class;
extern void Float_init_class();




