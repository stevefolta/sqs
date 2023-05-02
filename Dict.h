#pragma once

#include <stdint.h>

struct DictNode;
struct String;
struct Object;

typedef struct Dict {
	struct DictNode* tree;
	int capacity, size;
	} Dict;

extern Dict* new_Dict();
extern void Dict_init(Dict* self);
extern void Dict_set_at(Dict* self, struct String* key, struct Object* value);
extern struct Object* Dict_at(Dict* self, struct String* key);
extern struct String* Dict_key_at(Dict* self, struct String* key);
	// Useful to avoid proliferations of the same string.
extern void Dict_dump(Dict* self);



