/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>

#include "../util.h"

#define MAX_NUM_CORE 32

const char *
cpucore_perc(const char *fmt)
{
	static int cores;
	static long double b[MAX_NUM_CORE][7];
	static long double a[7];
	FILE *fp;
	int perc;
	int i;
	char *str = buf;

	if (!(fp = fopen("/proc/stat", "r"))) {
		warn("fopen /proc/stat: %s\n", strerror(errno));
		return NULL;
	}

	/* skip total */
	if (fscanf(fp, "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf 0 0 0", &a[0], &a[1], &a[2],
	           &a[3], &a[4], &a[5], &a[6]) != 7)
		goto fail;

	/* if run for the first time, count cores */
	if (cores == 0) {
		for (i = 0; ; i++) {
			if (fscanf(fp, "%s %Lf %Lf %Lf %Lf %Lf %Lf %Lf 0 0 0", buf,
			           &b[i][0], &b[i][1], &b[i][2], &b[i][3], &b[i][4], &b[i][5], &b[i][6]) != 8)
				goto fail;
			if (strncmp(buf, "cpu", 3) != 0)
				break;
		}
		cores = i;
		goto fail;
	}

	for (i = 0; i < cores; i++) {
		if (fscanf(fp, "%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf 0 0 0", &a[0], &a[1], &a[2],
				   &a[3], &a[4], &a[5], &a[6]) != 7)
			goto fail;

		perc = 100 * ((b[i][0]+b[i][1]+b[i][2]+b[i][5]+b[i][6]) - (a[0]+a[1]+a[2]+a[5]+a[6])) /
			   ((b[i][0]+b[i][1]+b[i][2]+b[i][3]+b[i][4]+b[i][5]+b[i][6]) - (a[0]+a[1]+a[2]+a[3]+a[4]+a[5]+a[6]));

		memcpy(b[i], a, sizeof(a));
		
		str += sprintf(str, fmt, perc);
	}

	fclose(fp);
	return buf;

fail:
	fclose(fp);
	return NULL;
}
