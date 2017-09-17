/* See LICENSE file for copyright and license details. */
#include <err.h>
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>

#include "arg.h"
#include "util.h"

struct arg {
	const char *(*func)();
	const char *fmt;
	const char *args;
};

static const char *battery_perc(const char *bat);
static const char *battery_power(const char *bat);
static const char *battery_state(const char *bat);
static const char *cpu_freq(void);
static const char *cpu_perc(void);
static const char *cpu_iowait(void);
static const char *datetime(const char *fmt);
static const char *disk_free(const char *mnt);
static const char *disk_perc(const char *mnt);
static const char *disk_total(const char *mnt);
static const char *disk_used(const char *mnt);
static const char *entropy(void);
static const char *gid(void);
static const char *hostname(void);
static const char *ipv4(const char *iface);
static const char *ipv6(const char *iface);
static const char *kernel_release(void);
static const char *keyboard_indicators(void);
static const char *load_avg(const char *fmt);
static const char *num_files(const char *dir);
static const char *ram_free(void);
static const char *ram_perc(void);
static const char *ram_used(void);
static const char *ram_total(void);
static const char *run_command(const char *cmd);
static const char *swap_free(void);
static const char *swap_perc(void);
static const char *swap_used(void);
static const char *swap_total(void);
static const char *temp(const char *file);
static const char *uid(void);
static const char *uptime(void);
static const char *username(void);
static const char *vol_perc(const char *card);
static const char *wifi_perc(const char *iface);
static const char *wifi_essid(const char *iface);

char *argv0;
static unsigned short int done;
static Display *dpy;

#include "config.h"

static void
terminate(const int signo)
{
	done = 1;
}

static void
difftimespec(struct timespec *res, struct timespec *a, struct timespec *b)
{
	res->tv_sec = a->tv_sec - b->tv_sec - (a->tv_nsec < b->tv_nsec);
	res->tv_nsec = a->tv_nsec - b->tv_nsec +
	               (a->tv_nsec < b->tv_nsec) * 1000000000;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-s]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct sigaction act;
	struct timespec start, current, diff, intspec, wait;
	size_t i, len;
	int sflag = 0;
	char status[MAXLEN];

	ARGBEGIN {
		case 's':
			sflag = 1;
			break;
		default:
			usage();
	} ARGEND

	if (argc) {
		usage();
	}

	setlocale(LC_ALL, "");

	memset(&act, 0, sizeof(act));
	act.sa_handler = terminate;
	sigaction(SIGINT,  &act, NULL);
	sigaction(SIGTERM, &act, NULL);

	if (!sflag && !(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "slstatus: cannot open display");
		return 1;
	}

	while (!done) {
		clock_gettime(CLOCK_MONOTONIC, &start);

		status[0] = '\0';
		for (i = len = 0; i < LEN(args); i++) {
			len += snprintf(status + len, sizeof(status) - len,
			                args[i].fmt, args[i].func(args[i].args));

			if (len >= sizeof(status)) {
				status[sizeof(status) - 1] = '\0';
			}
		}

		if (sflag) {
			printf("%s\n", status);
		} else {
			XStoreName(dpy, DefaultRootWindow(dpy), status);
			XSync(dpy, False);
		}

		if (!done) {
			clock_gettime(CLOCK_MONOTONIC, &current);
			difftimespec(&diff, &current, &start);

			intspec.tv_sec = interval / 1000;
			intspec.tv_nsec = (interval % 1000) * 1000000;
			difftimespec(&wait, &intspec, &diff);

			if (wait.tv_sec >= 0) {
				nanosleep(&wait, NULL);
			}
		}
	}

	if (!sflag) {
		XStoreName(dpy, DefaultRootWindow(dpy), NULL);
		XCloseDisplay(dpy);
	}

	return 0;
}
