#pragma once

struct Object;

struct Object* Sleep(struct Object* self, struct Object** args);
struct Object* Getpid(struct Object* self, struct Object** args);
struct Object* Get_cwd(struct Object* self, struct Object** args);
struct Object* Chdir(struct Object* self, struct Object** args);
struct Object* Rename(struct Object* self, struct Object** args);

