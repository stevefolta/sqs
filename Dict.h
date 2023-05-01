#pragma once

#include "Class.h"
#include "Object.h"
#include "Memory.h"
#include <stdlib.h>

struct String;

typedef struct Dict {
	Class* class_;
	size_t size, capacity;
	struct Dict_KV* items;
	} Dict;

extern Object* Dict_init(Dict* self);
extern Object* Dict_at(struct Dict* self, struct String* key);
extern Object* Dict_set_at(struct Dict* self, struct String* key, struct Object* value);


inline Dict* new_Dict()
{
	Dict* self = (Dict*) alloc_mem(sizeof(Dict));
	Dict_init(self);
	return self;
}

