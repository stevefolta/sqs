#pragma once

#include "gc.h"

#define alloc_mem(size) (GC_MALLOC(size))
#define realloc_mem(ptr, size) (GC_REALLOC(ptr, size))

