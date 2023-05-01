#pragma once

struct Method;

typedef struct MethodBuilder {
	struct Method* method;
	int cur_num_variables, max_num_variables;
	} MethodBuilder;

extern MethodBuilder* new_MethodBuilder();

