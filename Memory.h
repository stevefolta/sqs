#pragma once

#include "gc.h"

// Note that these will clear the allocated memory.

#define alloc_mem(size) (GC_MALLOC(size))
#define realloc_mem(ptr, size) (GC_REALLOC(ptr, size))

#define alloc_obj(Type) ((Type*) alloc_mem(sizeof(Type)))

