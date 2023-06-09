#pragma once

#include <regex.h>

struct Class;
struct Dict;


typedef struct Regex {
	struct Class* class_;
	regex_t* regex;
	int num_groups;
	struct Dict* capture_groups;
	} Regex;

extern struct Class Regex_class;
extern void Regex_init_class();

