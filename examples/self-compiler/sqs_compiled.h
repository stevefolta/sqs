#pragma once

#include "Object.h"
#include "Class.h"
#include "String.h"
#include "Int.h"
#include "Float.h"
#include "Array.h"
#include "Dict.h"
#include "Boolean.h"
#include "ByteArray.h"
#include "Nil.h"
#include "File.h"
#include "Pipe.h"
#include "Path.h"
#include "BuiltinMethod.h"

#include "Print.h"
#include "Run.h"
#include "Regex.h"
#include "Glob.h"
#include "MiscFunctions.h"
#include "Fail.h"
#include "Env.h"


typedef Object* Object_ptr;

#define ivar_get_(index) ( ((Object**) self)[index + 1] )
#define ivar_set_(index, value) ( ((Object**) self)[index + 1] = value )

extern Object* call_(const char* name, Object* receiver, int num_args, Object** args);
extern Object* super_call_(const char* name, Object* receiver, int num_args, Object** args);

typedef struct UpvalFrame UpvalFrame;
struct UpvalFrame {
	int capture_id;
	UpvalFrame* up;
	Object*** captures;
	};
extern Object** get_upvalue_(int capture_id, int index);
extern UpvalFrame* cur_upval_frame_;

extern Array argv_obj;

#define inline_string_(value) ((Object*) &(String) { &String_class, value, sizeof(value) - 1 })

