#pragma once

struct ByteArray;
struct Array;


typedef struct Method {
	struct ByteArray* bytecode;
	struct Array* literals;
	} Method;

Method* new_Method();


