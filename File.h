#pragma once

#include <stdio.h>

struct File;
struct Class;
struct Object;
struct File;

extern struct Class File_class;
void File_init_class();

FILE* File_get_file(struct File* file);
int File_fd(struct File* file);
struct Object* File_flush(struct Object* super, struct Object** args);


// Helpers.
extern struct String* file_contents(const char* file_path);


