#pragma once

#include <stdint.h>

struct Method;
struct Environment;
struct Object;


typedef struct MethodBuilder {
	struct Method* method;
	int cur_num_variables, max_num_variables;
	struct Environment* environment;
	} MethodBuilder;

extern MethodBuilder* new_MethodBuilder();
extern int MethodBuilder_add_literal(MethodBuilder* self, struct Object* literal);

extern void MethodBuilder_add_bytecode(MethodBuilder* self, uint8_t bytecode);
extern int MethodBuilder_add_offset8(MethodBuilder* self);
extern void MethodBuilder_add_back_offset8(MethodBuilder* self, int patch_point);
extern void MethodBuilder_patch_offset8(MethodBuilder* self, int patch_point);
extern int MethodBuilder_get_offset(MethodBuilder* self);

extern int MethodBuilder_reserve_locals(MethodBuilder* self, int num_locals);
extern void MethodBuilder_release_locals(MethodBuilder* self, int num_locals);

