# See LICENSE file for copyright and license details
# slstatus - suckless status monitor
.POSIX:

include config.mk

REQ = util
HDR = arg.h
COM =\
	battery\
	cpu\
	datetime\
	disk\
	entropy\
	hostname\
	ip\
	kernel_release\
	keyboard_indicators\
	load_avg\
	num_files\
	ram\
	run_command\
	swap\
	temperature\
	uptime\
	user\
	volume\
	wifi

all: slstatus

slstatus: slstatus.o $(COM:=.o) $(REQ:=.o)
slstatus.o: slstatus.c slstatus.h $(HDR) $(REQ:=.h)

battery.o: battery.c config.mk $(HDR) $(REQ:=.h)
cpu.o: cpu.c config.mk $(HDR) $(REQ:=.h)
datetime.o: datetime.c config.mk $(HDR) $(REQ:=.h)
disk.o: disk.c config.mk $(HDR) $(REQ:=.h)
entropy.o: entropy.c config.mk $(HDR) $(REQ:=.h)
hostname.o: hostname.c config.mk $(HDR) $(REQ:=.h)
ip.o: ip.c config.mk $(HDR) $(REQ:=.h)
kernel_release.o: kernel_release.c config.mk $(HDR) $(REQ:=.h)
keyboard_indicators.o: keyboard_indicators.c config.mk $(HDR) $(REQ:=.h)
load_avg.o: load_avg.c config.mk $(HDR) $(REQ:=.h)
num_files.o: num_files.c config.mk $(HDR) $(REQ:=.h)
ram.o: ram.c config.mk $(HDR) $(REQ:=.h)
run_command.o: run_command.c config.mk $(HDR) $(REQ:=.h)
swap.o: swap.c config.mk $(HDR) $(REQ:=.h)
temperature.o: temperature.c config.mk $(HDR) $(REQ:=.h)
uptime.o: uptime.c config.mk $(HDR) $(REQ:=.h)
user.o: user.c config.mk $(HDR) $(REQ:=.h)
volume.o: volume.c config.mk $(HDR) $(REQ:=.h)
wifi.o: wifi.c config.mk $(HDR) $(REQ:=.h)

.o:
	$(CC) -o $@ $(LDFLAGS) $< $(COM:=.o) $(REQ:=.o) $(LDLIBS)

.c.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

clean:
	rm -f slstatus slstatus.o $(COM:=.o) $(REQ:=.o)

dist:
	rm -rf "slstatus-$(VERSION)"
	mkdir -p "slstatus-$(VERSION)"
	cp -R LICENSE Makefile README config.mk config.def.h \
	      $(HDR) slstatus.c $(COM:=.c) $(REQ:=.c) $(REQ:=.h) \
	      slstatus.1 "slstatus-$(VERSION)"
	tar -cf - "slstatus-$(VERSION)" | gzip -c > "slstatus-$(VERSION).tar.gz"
	rm -rf "slstatus-$(VERSION)"

install: all
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp -f slstatus "$(DESTDIR)$(PREFIX)/bin"
	chmod 755 "$(DESTDIR)$(PREFIX)/bin/slstatus"
	mkdir -p "$(DESTDIR)$(MANPREFIX)/man1"
	cp -f slstatus.1 "$(DESTDIR)$(MANPREFIX)/man1"
	chmod 644 "$(DESTDIR)$(MANPREFIX)/man1/slstatus.1"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/slstatus"
	rm -f "$(DESTDIR)$(MANPREFIX)/man1/slstatus.1"
