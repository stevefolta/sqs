#include "Init.h"
#include "Object.h"
#include "Class.h"
#include "String.h"
#include "Int.h"
#include "Float.h"
#include "Boolean.h"
#include "Array.h"
#include "ByteArray.h"
#include "Dict.h"
#include "Nil.h"
#include "Method.h"
#include "BuiltinMethod.h"
#include "Environment.h"
#include "Print.h"
#include "Run.h"
#include "Pipe.h"
#include "Glob.h"
#include "File.h"
#include "LinesIterator.h"
#include "Regex.h"
#include "Path.h"
#include "Env.h"
#include "MiscFunctions.h"
#include "Fail.h"


void init_all()
{
	Class_init_class();
	Object_init_class();
	String_init_class();
	Int_init_class();
	Float_init_class();
	Boolean_init_class();
	Array_init_class();
	ByteArray_init_class();
	Dict_init_class();
	Method_init_class();
	BuiltinMethod_init_class();
	Nil_init_class();
	File_init_class();
	Pipe_init_class();
	LinesIterator_init_class();
	Regex_init_class();
	Path_init_class();
	Env_init_class();

	Run_init();

	GlobalEnvironment_init();
	GlobalEnvironment_add_fn("print", 2, Print);
	GlobalEnvironment_add_fn("run", 2, Run);
	GlobalEnvironment_add_fn("glob", 2, Glob);
	GlobalEnvironment_add_fn("sleep", 1, Sleep);
	GlobalEnvironment_add_fn("getpid", 1, Getpid);
	GlobalEnvironment_add_fn("fail", 1, Fail);
	GlobalEnvironment_add_fn("cwd", 0, Get_cwd);
	GlobalEnvironment_add_fn("chdir", 1, Chdir);
	GlobalEnvironment_add_fn("rename", 1, Rename);
	GlobalEnvironment_add_class(&Array_class);
	GlobalEnvironment_add_class(&ByteArray_class);
	GlobalEnvironment_add_class(&Dict_class);
	GlobalEnvironment_add_class(&String_class);
	GlobalEnvironment_add_class(&Int_class);
	GlobalEnvironment_add_class(&Float_class);
	GlobalEnvironment_add_class(&File_class);
	GlobalEnvironment_add_class(&Pipe_class);
	GlobalEnvironment_add_class(&Regex_class);
	GlobalEnvironment_add_class(&Path_class);
	GlobalEnvironment_add_c("env", (Object*) &env_obj);
}


