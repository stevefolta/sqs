#pragma once

#include "Class.h"
#include "Object.h"
#include <stdint.h>
#include <stddef.h>

struct String;


typedef struct ByteArray {
	Class* class_;
	size_t size, capacity;
	uint8_t* array;
	} ByteArray;


extern ByteArray* new_ByteArray();
extern Object* ByteArray_init(ByteArray* self);
extern uint8_t ByteArray_at(struct ByteArray* self, size_t index);
extern void ByteArray_set_at(struct ByteArray* self, size_t index, uint8_t value);
extern void ByteArray_append(struct ByteArray* self, uint8_t value);
extern void ByteArray_append_bytes(ByteArray* self, uint8_t* values, size_t size);
extern struct String* ByteArray_as_string(ByteArray* self);

extern Class ByteArray_class;
extern void ByteArray_init_class();
