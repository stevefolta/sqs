#pragma once

#include "gc.h"

// Note that these will clear the allocated memory.

#define alloc_mem(size) (GC_MALLOC(size))
#define alloc_mem_no_pointers(size) (GC_MALLOC_ATOMIC(size))
#define realloc_mem(ptr, size) (GC_REALLOC(ptr, size))

#define alloc_obj(Type) ((Type*) alloc_mem(sizeof(Type)))

#define mem_add_finalizer(ptr, fn, data) (GC_REGISTER_FINALIZER(ptr, fn, data, NULL, NULL))

