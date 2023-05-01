#pragma once

struct Method;
struct Object;

typedef struct MethodBuilder {
	struct Method* method;
	int cur_num_variables, max_num_variables;
	} MethodBuilder;

extern MethodBuilder* new_MethodBuilder();
extern int MethodBuilder_add_literal(MethodBuilder* self, struct Object* literal);

