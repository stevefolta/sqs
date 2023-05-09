#pragma once

#include <stdio.h>

struct File;
struct Class;

extern struct Class File_class;
void File_init_class();

FILE* File_get_file(struct File* file);

