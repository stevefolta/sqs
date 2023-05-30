#include "Glob.h"
#include "Object.h"
#include "String.h"
#include "Array.h"
#include "Dict.h"
#include "Error.h"
#include <glob.h>

static bool Glob_initialized = false;
static String mark_directories, sort, escape;
static String tilde;

static void Glob_init()
{
	String_init_static_c(&mark_directories, "mark-directories");
	String_init_static_c(&sort, "sort");
	String_init_static_c(&escape, "escape");
	String_init_static_c(&tilde, "tilde");

	Glob_initialized = true;
}


Object* Glob(Object* self, Object** args)
{
	Glob_init();
	String* pattern = String_enforce(args[0], "glob()");

	// Flags.
	int flags = 0;
	Dict* options = (Dict*) args[1];
	if (options && options->class_ == &Dict_class) {
		if (Dict_option_turned_on(options, &mark_directories))
			flags |= GLOB_MARK;
		if (Dict_option_turned_off(options, &sort))
			flags |= GLOB_NOSORT;
		if (Dict_option_turned_off(options, &escape))
			flags |= GLOB_NOESCAPE;
		if (Dict_option_turned_on(options, &tilde)) {
			// Not POSIX, a GNU extension, but musl supports it too.
			flags |= GLOB_TILDE;
			}
		}

	// Do it.
	Array* result = new_Array();
	glob_t found_files;
	int err = glob(String_c_str(pattern), flags, NULL, &found_files);
	if (err == GLOB_NOMATCH) {
		globfree(&found_files);
		return (Object*) result;
		}
	else if (err != 0)
		Error("glob() failed for some reason.");

	// Build the result.
	for (int i = 0; i < found_files.gl_pathc; ++i)
		Array_append(result, (Object*) new_c_String(found_files.gl_pathv[i]));

	globfree(&found_files);
	return (Object*) result;
}



