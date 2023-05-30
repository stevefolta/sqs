#pragma once

struct Class;

typedef struct Env {
	struct Class* class_;
	} Env;

extern Env env_obj;
extern struct Class Env_class;
extern void Env_init_class();

