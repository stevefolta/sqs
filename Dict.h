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

	Object* (*init)(struct Dict* self);
	Object* (*at)(struct Dict* self, struct String* key);
	Object* (*set_at)(struct Dict* self, struct String* key, struct Object* value);
	} Dict;

extern Object* Dict_init(Dict* self);

inline Dict* new_Dict()
{
	Dict* self = (Dict*) alloc_mem(sizeof(Dict));
	self->init = Dict_init;
	self->init(self);
	return self;
}

