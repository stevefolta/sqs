#include "Dict.h"
#include "String.h"
#include <string.h>

// Quick 'n' dirty implementation with O(N) access.

#define capacity_increment 16

extern Object* Dict_at(struct Dict* self, struct String* key);
extern Object* Dict_set_at(struct Dict* self, struct String* key, struct Object* value);

typedef struct Dict_KV {
	String* key;
	Object* value;
	} Dict_KV;


Object* Dict_init(struct Dict* self)
{
	self->class_ = NULL; 	// TODO
	self->size = self->capacity = 0;
	self->items = NULL;
	return (Object*) self;
}


Object* Dict_at(struct Dict* self, String* key)
{
	for (int i = 0; i < self->size; ++i) {
		if (key->equals(key, self->items[i].key))
			return self->items[i].value;
		}

	return NULL;
}


Object* Dict_set_at(struct Dict* self, String* key, Object* value)
{
	for (int i = 0; i < self->size; ++i) {
		if (key->equals(key, self->items[i].key)) {
			self->items[i].value = value;
			return value;
			}
		}

	// The key isn't in the Dict yet; add it.
	if (self->size >= self->capacity) {
		int old_capacity = self->capacity;
		self->capacity += capacity_increment;
		if (self->items) {
			self->items = (Dict_KV*) realloc_mem(self->items, self->capacity * sizeof(Dict_KV));
			memset(self->items + old_capacity, 0, (self->capacity - old_capacity) * sizeof(Dict_KV));
			}
		else {
			self->items = (Dict_KV*) alloc_mem(self->capacity * sizeof(Dict_KV));
			memset(self->items, 0, self->capacity * sizeof(Dict_KV));
			}
		}

	self->items[self->capacity].key = key;
	self->items[self->capacity].value = value;
	self->capacity += 1;

	return value;
}



