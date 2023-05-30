#include "Sleep.h"
#include "Int.h"
#include "Object.h"
#include "Error.h"
#include <time.h>
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



