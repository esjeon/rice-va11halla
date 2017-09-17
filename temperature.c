#include <stdio.h>

#include "util.h"

const char *
temp(const char *file)
{
	int temp;

	return (pscanf(file, "%d", &temp) == 1) ?
	       bprintf("%d", temp / 1000) : NULL;
}
