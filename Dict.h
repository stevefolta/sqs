#pragma once

#include <stdint.h>
#include <stdbool.h>

struct DictNode;
struct String;
struct Object;
struct Class;
struct Array;

typedef struct Dict {
	struct Class* class_;
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


typedef struct DictIteratorResult {
	struct String* key;
	struct Object* value;
	} DictIteratorResult;

typedef struct DictIterator {
	struct Class* class_;
	struct Dict* dict;
	struct Array* stack;
	} DictIterator;

struct DictIterator* new_DictIterator(Dict* dict);
DictIteratorResult DictIterator_next(DictIterator* self);


extern struct Class Dict_class;
extern void Dict_init_class();

// Helpers when used for function options.
bool Dict_option_turned_on(Dict* dict, struct String* option_name);
bool Dict_option_turned_off(Dict* dict, struct String* option_name);


