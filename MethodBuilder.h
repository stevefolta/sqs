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

extern MethodBuilder* new_MethodBuilder(int num_args);
extern void MethodBuilder_finish(MethodBuilder* self);
extern int MethodBuilder_add_literal(MethodBuilder* self, struct Object* literal);

extern void MethodBuilder_add_bytecode(MethodBuilder* self, uint8_t bytecode);
extern int MethodBuilder_add_offset8(MethodBuilder* self);
extern void MethodBuilder_add_back_offset8(MethodBuilder* self, int patch_point);
extern void MethodBuilder_patch_offset8(MethodBuilder* self, int patch_point);
extern int MethodBuilder_get_offset(MethodBuilder* self);

extern void MethodBuilder_add_move(MethodBuilder* self, int src, int dest);

extern int MethodBuilder_reserve_locals(MethodBuilder* self, int num_locals);
extern void MethodBuilder_release_locals(MethodBuilder* self, int num_locals);

extern void MethodBuilder_push_environment(MethodBuilder* self, struct Environment* environment);
extern void MethodBuilder_pop_environment(MethodBuilder* self);

