#include "MiscFunctions.h"
#include "Int.h"
#include "Float.h"
#include "Object.h"
#include "Class.h"
#include "String.h"
#include "Path.h"
#include "Nil.h"
#include "Memory.h"
#include "Error.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>


Object* Sleep(Object* self, Object** args)
{
	double seconds = Float_enforce(args[0], "sleep");
	struct timespec ts = { seconds, (seconds - (int) seconds) * 1000000000 };
	while (true) {
		int result = nanosleep(&ts, &ts);
		if (result == 0)
			break;
		if (errno != EINTR)
			Error("Error in sleep() (%s).", strerror(errno));
		}
	return NULL;
}


Object* Getpid(Object* self, Object** args)
{
	return (Object*) new_Int(getpid());
}


Object* Get_cwd(Object* self, Object** args)
{
	size_t buf_size = 64;
	char* buffer = NULL;
	while (true) {
		buffer = alloc_mem_no_pointers(buf_size);
		if (getcwd(buffer, buf_size) == NULL) {
			if (errno != ERANGE)
				Error("Error getting the cwd() (%s).", strerror(errno));
			}
		else
			break;
		buf_size += 64;
		}
	return (Object*) new_c_String(buffer);
}


const char* enforce_path(Object* object, const char* where)
{
	const char* c_str = NULL;
	if (object) {
		if (object->class_ == &String_class)
			c_str = String_c_str((String*) object);
		else if (object->class_ == &Path_class)
			c_str = ((Path*) object)->path;
		}
	if (c_str == NULL) {
		Class* class_ = (object ? object->class_ : &Nil_class);
		Error("String required, but got a %s, in \"%s\".", String_c_str(class_->name), where);
		}
	return c_str;
}

Object* Chdir(Object* self, Object** args)
{
	const char* new_wd = enforce_path(args[0], "chdir()");
	int result = chdir(new_wd);
	if (result != 0)
		Error("Failure to set working directory (%s).", strerror(errno));
	return args[0];
}


Object* Rename(Object* self, Object** args)
{
	const char* old_path = enforce_path(args[0], "rename old-path");
	const char* new_path = enforce_path(args[1], "rename new-path");
	int result = rename(old_path, new_path);
	if (result != 0)
		Error("rename() failed (%s)\n", strerror(errno));
	return NULL;
}



