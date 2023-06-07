#include "MiscFunctions.h"
#include "Int.h"
#include "Object.h"
#include "String.h"
#include "Memory.h"
#include "Error.h"
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>


Object* Sleep(Object* self, Object** args)
{
	int seconds = Int_enforce(args[0], "sleep");
	struct timespec ts = { seconds, 0 };
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


Object* Chdir(Object* self, Object** args)
{
	String* new_wd = String_enforce(args[0], "chdir()");
	int result = chdir(String_c_str(new_wd));
	if (result != 0)
		Error("Failure to set working directory (%s).", strerror(errno));
	return args[0];
}



