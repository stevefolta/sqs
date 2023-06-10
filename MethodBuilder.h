#pragma once

#include <stdint.h>

struct Method;
struct Environment;
struct Array;
struct Object;
struct String;


typedef struct MethodBuilder {
	struct Method* method;
	struct Array* arguments;
	int cur_num_variables, max_num_variables;
	struct Environment* environment;
	struct LoopPoints* loop_points;
	} MethodBuilder;

extern MethodBuilder* new_MethodBuilder(struct Array* arguments, struct Environment* environment);
extern void MethodBuilder_finish(MethodBuilder* self);
extern void MethodBuilder_finish_init(MethodBuilder* self);

extern int MethodBuilder_emit_literal(MethodBuilder* self, struct Object* literal);
extern int MethodBuilder_emit_literal_by_num(MethodBuilder* self, int literal_num);
extern int MethodBuilder_emit_string_literal(MethodBuilder* self, struct String* literal);
extern int MethodBuilder_add_literal(MethodBuilder* self, struct Object* literal);
extern int MethodBuilder_reserve_literal(MethodBuilder* self);
extern void MethodBuilder_set_literal(MethodBuilder* self, int literal, struct Object* value);

extern void MethodBuilder_add_bytecode(MethodBuilder* self, uint8_t bytecode);
extern int MethodBuilder_add_offset8(MethodBuilder* self);
extern void MethodBuilder_add_back_offset8(MethodBuilder* self, int patch_point);
extern void MethodBuilder_patch_offset8(MethodBuilder* self, int patch_point);
extern int MethodBuilder_add_offset16(MethodBuilder* self);
extern void MethodBuilder_add_back_offset16(MethodBuilder* self, int patch_point);
extern void MethodBuilder_patch_offset16(MethodBuilder* self, int patch_point);
extern void MethodBuilder_patch_offset16_to(MethodBuilder* self, int patch_point, int dest_point);
extern int MethodBuilder_get_offset(MethodBuilder* self);

extern void MethodBuilder_add_move(MethodBuilder* self, int src, int dest);

extern int MethodBuilder_reserve_locals(MethodBuilder* self, int num_locals);
extern void MethodBuilder_release_locals(MethodBuilder* self, int num_locals);
extern int MethodBuilder_find_argument(MethodBuilder* self, struct String* name);

extern void MethodBuilder_push_environment(MethodBuilder* self, struct Environment* environment);
extern void MethodBuilder_pop_environment(MethodBuilder* self);

extern void MethodBuilder_push_loop_points(MethodBuilder* self);
extern void MethodBuilder_pop_loop_points(MethodBuilder* self, int continue_point, int break_point);
extern void MethodBuilder_add_continue_offset8(MethodBuilder* self);
extern void MethodBuilder_add_break_offset8(MethodBuilder* self);
extern void MethodBuilder_add_continue_offset16(MethodBuilder* self);
extern void MethodBuilder_add_break_offset16(MethodBuilder* self);

