#pragma once

struct Object;
struct String;

struct Object* Run(struct Object* self, struct Object** args);

extern void Run_init();

extern struct String capture_string;
extern struct String wait_string;
extern struct String stdin_string;
extern struct String stdout_string;
extern struct String stderr_string;
extern struct String env_string;

