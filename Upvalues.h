#pragma once

#include "ParseNode.h"

struct Block;
struct Method;
struct MethodBuilder;

// We don't have closures per se, so upvals don't need to outlive their stack
// frames.  We do upvals by just walking the stack to find the enclosing frame,
// and accessing the local there.  Not the most efficient thing, but probably
// not bad in most situations.


typedef struct UpvalueLocal {
	ParseNode parse_node;
	struct Method* method;
	int local_index;
	} UpvalueLocal;
extern UpvalueLocal* new_UpvalueLocal(struct Method* method, int local_index);

typedef struct UpvalueLocalPatchPoint {
	struct MethodBuilder* method_builder;
	int patch_point;
	} UpvalueLocalPatchPoint;

