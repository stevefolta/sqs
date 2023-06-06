#pragma once

struct Object;
struct String;

struct Object* Run(struct Object* self, struct Object** args);

extern void Run_init();

extern struct String capture_string;
extern struct String wait_string;
extern struct String stdin_pipe_string;
extern struct String stdout_pipe_string;
extern struct String stderr_pipe_string;
extern struct String env_string;

