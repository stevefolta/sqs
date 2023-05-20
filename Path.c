#include "Path.h"
#include "Class.h"
#include "String.h"
#include "Boolean.h"
#include "Int.h"
#include "Object.h"
#include "Memory.h"
#include "Error.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

Class Path_class;


Object* Path_init(Object* super, Object** args)
{
	Path* self = (Path*) super;

	// Set up "self->path", doing basic tilde expansion.
	// We'll be using the path with C calls, so we'll include a trailing null
	// byte.
	String* initial_path = String_enforce(args[0], "Path.init");
	if (initial_path->size > 0 && initial_path->str[0] == '~') {
		char* home_dir = getenv("HOME");
		if (home_dir == NULL)
			Error("\"~\" used in a Path, but $HOME isn't set.");
		size_t home_dir_size = strlen(home_dir);
		if (initial_path->size == 1) {
			self->path = alloc_mem(home_dir_size + 1);
			memcpy(self->path, home_dir, home_dir_size);
			self->path[home_dir_size] = 0;
			}
		else if (initial_path->str[1] == '/') {
			self->path = alloc_mem(home_dir_size + initial_path->size - 1 + 1);
			memcpy(self->path, home_dir, home_dir_size);
			memcpy(self->path + home_dir_size, &initial_path->str[1], initial_path->size - 1);
			self->path[home_dir_size + initial_path->size - 1] = 0;
			}
		else
			Error("Only basic tilde expansion is available in Path (\"%s\").", String_c_str(initial_path));
		}
	else {
		self->path = alloc_mem(initial_path->size + 1);
		memcpy(self->path, initial_path->str, initial_path->size);
		self->path[initial_path->size] = 0;
		}

	return super;
}


Object* Path_string(Object* super, Object** args)
{
	Path* self = (Path*) super;
	return (Object*) new_static_String(self->path, strlen(self->path));
}

Object* Path_exists(Object* super, Object** args)
{
	Path* self = (Path*) super;

	struct stat info;
	int result = stat(self->path, &info);
	if (result != 0) {
		if (errno == ENOENT)
			return &false_obj;
		Error("Error when getting info about \"%s\" (%s).", self->path, strerror(errno));
		}
	return &true_obj;
}

Object* Path_is_file(Object* super, Object** args)
{
	Path* self = (Path*) super;

	struct stat info;
	int result = stat(self->path, &info);
	if (result != 0) {
		if (errno == ENOENT)
			return &false_obj;
		Error("Error when getting info about \"%s\" (%s).", self->path, strerror(errno));
		}
	return make_bool(S_ISREG(info.st_mode));
}

Object* Path_is_dir(Object* super, Object** args)
{
	Path* self = (Path*) super;

	struct stat info;
	int result = stat(self->path, &info);
	if (result != 0) {
		if (errno == ENOENT)
			return &false_obj;
		Error("Error when getting info about \"%s\" (%s).", self->path, strerror(errno));
		}
	return make_bool(S_ISDIR(info.st_mode));
}

Object* Path_is_symlink(Object* super, Object** args)
{
	Path* self = (Path*) super;

	struct stat info;
	int result = lstat(self->path, &info);
	if (result != 0) {
		if (errno == ENOENT)
			return &false_obj;
		Error("Error when getting info about \"%s\" (%s).", self->path, strerror(errno));
		}
	return make_bool(S_ISLNK(info.st_mode));
}

Object* Path_size(Object* super, Object** args)
{
	Path* self = (Path*) super;

	struct stat info;
	int result = stat(self->path, &info);
	if (result != 0)
		Error("Error when getting info about \"%s\" (%s).", self->path, strerror(errno));
	return (Object*) new_Int(info.st_size);
}


void Path_init_class()
{
	init_static_class(Path);

	static const BuiltinMethodSpec builtin_methods[] = {
		{ "init", 2, Path_init },
		{ "string", 0, Path_string },
		{ "exists", 0, Path_exists },
		{ "is-file", 0, Path_is_file },
		{ "is-dir", 0, Path_is_dir },
		{ "is-symlink", 0, Path_is_symlink },
		{ "size", 0, Path_size },
		{ NULL },
		};
	Class_add_builtin_methods(&Path_class, builtin_methods);
}




