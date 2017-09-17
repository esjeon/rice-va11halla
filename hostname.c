#include <err.h>
#include <unistd.h>

#include "util.h"

const char *
hostname(void)
{
	if (gethostname(buf, sizeof(buf)) == -1) {
		warn("hostname");
		return NULL;
	}

	return buf;
}
