/* See LICENSE file for copyright and license details. */
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <limits.h>
#include <linux/wireless.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/socket.h>
#include <sys/soundcard.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "arg.h"

#define LEN(x) (sizeof (x) / sizeof *(x))

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
static const char *datetime(const char *fmt);
static const char *disk_free(const char *mnt);
static const char *disk_perc(const char *mnt);
static const char *disk_total(const char *mnt);
static const char *disk_used(const char *mnt);
static const char *entropy(void);
static const char *gid(void);
static const char *hostname(void);
static const char *ip(const char *iface);
static const char *kernel_release(void);
static const char *keyboard_indicators(void);
static const char *load_avg(void);
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

static char buf[MAXLEN];

static const char *
bprintf(const char *fmt, ...)
{
	va_list ap;
	size_t len;

	va_start(ap, fmt);
	len = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	va_end(ap);

	if (len >= sizeof(buf))
		buf[sizeof(buf)-1] = '\0';

	return buf;
}

int
pscanf(const char *path, const char *fmt, ...)
{
	FILE *fp;
	va_list ap;
	int n;

	if (!(fp = fopen(path, "r"))) {
		warn("fopen %s: %s\n", path, strerror(errno));
		return -1;
	}
	va_start(ap, fmt);
	n = vfscanf(fp, fmt, ap);
	va_end(ap);
	fclose(fp);

	return (n == EOF) ? -1 : n;
}

static const char *
battery_perc(const char *bat)
{
	int perc;
	char path[PATH_MAX];

	snprintf(path, sizeof(path), "%s%s%s", "/sys/class/power_supply/", bat, "/capacity");
	return (pscanf(path, "%i", &perc) == 1) ?
	       bprintf("%d", perc) : unknown_str;
}

static const char *
battery_power(const char *bat)
{
	int watts;
	char path[PATH_MAX];

	snprintf(path, sizeof(path), "%s%s%s", "/sys/class/power_supply/", bat, "/power_now");
	return (pscanf(path, "%i", &watts) == 1) ?
	       bprintf("%d", (watts + 500000) / 1000000) : unknown_str;
}

static const char *
battery_state(const char *bat)
{
	struct {
		char *state;
		char *symbol;
	} map[] = {
		{ "Charging",    "+" },
		{ "Discharging", "-" },
		{ "Full",        "=" },
		{ "Unknown",     "/" },
	};
	size_t i;
	char path[PATH_MAX], state[12];

	snprintf(path, sizeof(path), "%s%s%s", "/sys/class/power_supply/", bat, "/status");
	if (pscanf(path, "%12s", state) != 1) {
		return unknown_str;
	}

	for (i = 0; i < LEN(map); i++) {
		if (!strcmp(map[i].state, state)) {
			break;
		}
	}
	return (i == LEN(map)) ? "?" : map[i].symbol;
}

static const char *
cpu_freq(void)
{
	int freq;

	return (pscanf("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq",
	               "%i", &freq) == 1) ?
	       bprintf("%d", (freq + 500) / 1000) : unknown_str;
}

static const char *
cpu_perc(void)
{
	struct timespec delay;
	int perc;
	long double a[4], b[4];

	if (pscanf("/proc/stat", "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2],
	           &a[3]) != 4) {
		return unknown_str;
	}

	delay.tv_sec = (interval / 2) / 1000;
	delay.tv_nsec = ((interval / 2) % 1000) * 1000000;
	nanosleep(&delay, NULL);

	if (pscanf("/proc/stat", "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2],
	           &b[3]) != 4) {
		return unknown_str;
	}

	perc = 100 * ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) /
	       ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));

	return bprintf("%d", perc);
}

static const char *
datetime(const char *fmt)
{
	time_t t;

	t = time(NULL);
	if (strftime(buf, sizeof(buf), fmt, localtime(&t)) == 0)
		return unknown_str;

	return buf;
}

static const char *
disk_free(const char *mnt)
{
	struct statvfs fs;

	if (statvfs(mnt, &fs) < 0) {
		warn("Failed to get filesystem info");
		return unknown_str;
	}

	return bprintf("%f", (float)fs.f_bsize * (float)fs.f_bfree / 1024 / 1024 / 1024);
}

static const char *
disk_perc(const char *mnt)
{
	int perc;
	struct statvfs fs;

	if (statvfs(mnt, &fs) < 0) {
		warn("Failed to get filesystem info");
		return unknown_str;
	}

	perc = 100 * (1.0f - ((float)fs.f_bfree / (float)fs.f_blocks));

	return bprintf("%d", perc);
}

static const char *
disk_total(const char *mnt)
{
	struct statvfs fs;

	if (statvfs(mnt, &fs) < 0) {
		warn("Failed to get filesystem info");
		return unknown_str;
	}

	return bprintf("%f", (float)fs.f_bsize * (float)fs.f_blocks / 1024 / 1024 / 1024);
}

static const char *
disk_used(const char *mnt)
{
	struct statvfs fs;

	if (statvfs(mnt, &fs) < 0) {
		warn("Failed to get filesystem info");
		return unknown_str;
	}

	return bprintf("%f", (float)fs.f_bsize * ((float)fs.f_blocks - (float)fs.f_bfree) / 1024 / 1024 / 1024);
}

static const char *
entropy(void)
{
	int num;

        return (pscanf("/proc/sys/kernel/random/entropy_avail", "%d", &num) == 1) ?
	               bprintf("%d", num) : unknown_str;
}

static const char *
gid(void)
{
	return bprintf("%d", getgid());
}

static const char *
hostname(void)
{
	if (gethostname(buf, sizeof(buf)) == -1) {
		warn("hostname");
		return unknown_str;
	}

	return buf;
}

static const char *
ip(const char *iface)
{
	struct ifaddrs *ifaddr, *ifa;
	int s;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1) {
		warn("Failed to get IP address for interface %s", iface);
		return unknown_str;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL) {
			continue;
		}
		s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if ((strcmp(ifa->ifa_name, iface) == 0) && (ifa->ifa_addr->sa_family == AF_INET)) {
			if (s != 0) {
				warnx("Failed to get IP address for interface %s", iface);
				return unknown_str;
			}
			return bprintf("%s", host);
		}
	}

	freeifaddrs(ifaddr);

	return unknown_str;
}

static const char *
kernel_release(void)
{
	struct utsname udata;

	if (uname(&udata) < 0) {
		return unknown_str;
	}

	return bprintf("%s", udata.release);
}

static const char *
keyboard_indicators(void)
{
	Display *dpy = XOpenDisplay(NULL);
	XKeyboardState state;

	if (dpy == NULL) {
		warnx("XOpenDisplay failed");
		return unknown_str;
	}
	XGetKeyboardControl(dpy, &state);
	XCloseDisplay(dpy);

	switch (state.led_mask) {
		case 1:
			return "c";
		case 2:
			return "n";
		case 3:
			return "cn";
		default:
			return "";
	}
}

static const char *
load_avg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		warnx("Failed to get the load avg");
		return unknown_str;
	}

	return bprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}

static const char *
num_files(const char *dir)
{
	struct dirent *dp;
	DIR *fd;
	int num = 0;

	if ((fd = opendir(dir)) == NULL) {
		warn("Failed to get number of files in directory %s", dir);
		return unknown_str;
	}

	while ((dp = readdir(fd)) != NULL) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue; /* skip self and parent */
		num++;
	}

	closedir(fd);

	return bprintf("%d", num);
}

static const char *
ram_free(void)
{
	long free;

	return (pscanf("/proc/meminfo", "MemFree: %ld kB\n", &free) == 1) ?
	       bprintf("%f", (float)free / 1024 / 1024) : unknown_str;
}

static const char *
ram_perc(void)
{
	long total, free, buffers, cached;

	return (pscanf("/proc/meminfo",
	               "MemTotal: %ld kB\n"
	               "MemFree: %ld kB\n"
	               "MemAvailable: %ld kB\nBuffers: %ld kB\n"
	               "Cached: %ld kB\n",
	               &total, &free, &buffers, &buffers, &cached) == 5) ?
	       bprintf("%d", 100 * ((total - free) - (buffers + cached)) / total) :
	       unknown_str;
}

static const char *
ram_total(void)
{
	long total;

	return (pscanf("/proc/meminfo", "MemTotal: %ld kB\n", &total) == 1) ?
	       bprintf("%f", (float)total / 1024 / 1024) : unknown_str;
}

static const char *
ram_used(void)
{
	long total, free, buffers, cached;

	return (pscanf("/proc/meminfo",
	               "MemTotal: %ld kB\n"
	               "MemFree: %ld kB\n"
	               "MemAvailable: %ld kB\nBuffers: %ld kB\n"
	               "Cached: %ld kB\n",
	               &total, &free, &buffers, &buffers, &cached) == 5) ?
	       bprintf("%f", (float)(total - free - buffers - cached) / 1024 / 1024) :
	       unknown_str;
}

static const char *
run_command(const char *cmd)
{
	char *p;
	FILE *fp;

	fp = popen(cmd, "r");
	if (fp == NULL) {
		warn("Failed to get command output for %s", cmd);
		return unknown_str;
	}
	p = fgets(buf, sizeof(buf) - 1, fp);
	pclose(fp);
	if (!p)
		return unknown_str;
	if ((p = strrchr(buf, '\n')) != NULL)
		p[0] = '\0';

	return buf[0] ? buf : unknown_str;
}

static const char *
swap_free(void)
{
	long total, free;
	FILE *fp;
	size_t bytes_read;
	char *match;

	fp = fopen("/proc/meminfo", "r");
	if (fp == NULL) {
		warn("Failed to open file /proc/meminfo");
		return unknown_str;
	}

	if ((bytes_read = fread(buf, sizeof(char), sizeof(buf) - 1, fp)) == 0) {
		warn("swap_free: read error");
		fclose(fp);
		return unknown_str;
	}
	fclose(fp);

	if ((match = strstr(buf, "SwapTotal")) == NULL)
		return unknown_str;
	sscanf(match, "SwapTotal: %ld kB\n", &total);

	if ((match = strstr(buf, "SwapFree")) == NULL)
		return unknown_str;
	sscanf(match, "SwapFree: %ld kB\n", &free);

	return bprintf("%f", (float)free / 1024 / 1024);
}

static const char *
swap_perc(void)
{
	long total, free, cached;
	FILE *fp;
	size_t bytes_read;
	char *match;

	fp = fopen("/proc/meminfo", "r");
	if (fp == NULL) {
		warn("Failed to open file /proc/meminfo");
		return unknown_str;
	}

	if ((bytes_read = fread(buf, sizeof(char), sizeof(buf) - 1, fp)) == 0) {
		warn("swap_perc: read error");
		fclose(fp);
		return unknown_str;
	}
	fclose(fp);

	if ((match = strstr(buf, "SwapTotal")) == NULL)
		return unknown_str;
	sscanf(match, "SwapTotal: %ld kB\n", &total);

	if ((match = strstr(buf, "SwapCached")) == NULL)
		return unknown_str;
	sscanf(match, "SwapCached: %ld kB\n", &cached);

	if ((match = strstr(buf, "SwapFree")) == NULL)
		return unknown_str;
	sscanf(match, "SwapFree: %ld kB\n", &free);

	return bprintf("%d", 100 * (total - free - cached) / total);
}

static const char *
swap_total(void)
{
	long total;
	FILE *fp;
	size_t bytes_read;
	char *match;

	fp = fopen("/proc/meminfo", "r");
	if (fp == NULL) {
		warn("Failed to open file /proc/meminfo");
		return unknown_str;
	}
	if ((bytes_read = fread(buf, sizeof(char), sizeof(buf) - 1, fp)) == 0) {
		warn("swap_total: read error");
		fclose(fp);
		return unknown_str;
	}
	fclose(fp);

	if ((match = strstr(buf, "SwapTotal")) == NULL)
		return unknown_str;
	sscanf(match, "SwapTotal: %ld kB\n", &total);

	return bprintf("%f", (float)total / 1024 / 1024);
}

static const char *
swap_used(void)
{
	long total, free, cached;
	FILE *fp;
	size_t bytes_read;
	char *match;

	fp = fopen("/proc/meminfo", "r");
	if (fp == NULL) {
		warn("Failed to open file /proc/meminfo");
		return unknown_str;
	}
	if ((bytes_read = fread(buf, sizeof(char), sizeof(buf) - 1, fp)) == 0) {
		warn("swap_used: read error");
		fclose(fp);
		return unknown_str;
	}
	fclose(fp);

	if ((match = strstr(buf, "SwapTotal")) == NULL)
		return unknown_str;
	sscanf(match, "SwapTotal: %ld kB\n", &total);

	if ((match = strstr(buf, "SwapCached")) == NULL)
		return unknown_str;
	sscanf(match, "SwapCached: %ld kB\n", &cached);

	if ((match = strstr(buf, "SwapFree")) == NULL)
		return unknown_str;
	sscanf(match, "SwapFree: %ld kB\n", &free);

	return bprintf("%f", (float)(total - free - cached) / 1024 / 1024);
}

static const char *
temp(const char *file)
{
	int temp;

	return (pscanf(file, "%d", &temp) == 1) ?
	       bprintf("%d", temp / 1000) : unknown_str;
}

static const char *
uptime(void)
{
	struct sysinfo info;
	int h = 0;
	int m = 0;

	sysinfo(&info);
	h = info.uptime / 3600;
	m = (info.uptime - h * 3600 ) / 60;

	return bprintf("%dh %dm", h, m);
}

static const char *
username(void)
{
	struct passwd *pw = getpwuid(geteuid());

	if (pw == NULL) {
		warn("Failed to get username");
		return unknown_str;
	}

	return bprintf("%s", pw->pw_name);
}

static const char *
uid(void)
{
	return bprintf("%d", geteuid());
}


static const char *
vol_perc(const char *card)
{
	unsigned int i;
	int v, afd, devmask;
	char *vnames[] = SOUND_DEVICE_NAMES;

	afd = open(card, O_RDONLY | O_NONBLOCK);
	if (afd == -1) {
		warn("Cannot open %s", card);
		return unknown_str;
	}

	if (ioctl(afd, SOUND_MIXER_READ_DEVMASK, &devmask) == -1) {
		warn("Cannot get volume for %s", card);
		close(afd);
		return unknown_str;
	}
	for (i = 0; i < LEN(vnames); i++) {
		if (devmask & (1 << i) && !strcmp("vol", vnames[i])) {
			if (ioctl(afd, MIXER_READ(i), &v) == -1) {
				warn("vol_perc: ioctl");
				close(afd);
				return unknown_str;
			}
		}
	}

	close(afd);

	return bprintf("%d", v & 0xff);
}

static const char *
wifi_perc(const char *iface)
{
	int i, perc;
	char *p, *datastart;
	char path[PATH_MAX];
	char status[5];
	FILE *fp;

	snprintf(path, sizeof(path), "%s%s%s", "/sys/class/net/", iface, "/operstate");
	fp = fopen(path, "r");
	if (fp == NULL) {
		warn("Failed to open file %s", path);
		return unknown_str;
	}
	p = fgets(status, 5, fp);
	fclose(fp);
	if(!p || strcmp(status, "up\n") != 0) {
		return unknown_str;
	}

	fp = fopen("/proc/net/wireless", "r");
	if (fp == NULL) {
		warn("Failed to open file /proc/net/wireless");
		return unknown_str;
	}

	for (i = 0; i < 3; i++) {
		if (!(p = fgets(buf, sizeof(buf) - 1, fp)))
			break;
	}
	fclose(fp);
	if (i < 2 || !p)
		return unknown_str;

	if ((datastart = strstr(buf, iface)) == NULL)
		return unknown_str;

	datastart = (datastart+(strlen(iface)+1));
	sscanf(datastart + 1, " %*d   %d  %*d  %*d		  %*d	   %*d		%*d		 %*d	  %*d		 %*d", &perc);

	return bprintf("%d", perc);
}

static const char *
wifi_essid(const char *iface)
{
	static char id[IW_ESSID_MAX_SIZE+1];
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	struct iwreq wreq;

	memset(&wreq, 0, sizeof(struct iwreq));
	wreq.u.essid.length = IW_ESSID_MAX_SIZE+1;
	snprintf(wreq.ifr_name, sizeof(wreq.ifr_name), "%s", iface);

	if (sockfd == -1) {
		warn("Failed to get ESSID for interface %s", iface);
		return unknown_str;
	}
	wreq.u.essid.pointer = id;
	if (ioctl(sockfd,SIOCGIWESSID, &wreq) == -1) {
		warn("Failed to get ESSID for interface %s", iface);
		return unknown_str;
	}

	close(sockfd);

	if (strcmp(id, "") == 0)
		return unknown_str;
	else
		return id;
}

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
