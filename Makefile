
PREFIX = $(HOME)/.local/
CC = gcc

PARTS = dwm dmenu slstatus

.PHONY: all clean install uninstall $(PARTS)

all: $(PARTS)

$(PARTS):
	@echo 'MAKE $@'
	@make -C $@ -j CC="$(CC)"

clean:
	@for part in $(PARTS); do \
		make -C $$part clean; \
	done

install:
	@mkdir -p "$(PREFIX)"
	@for part in $(PARTS); do \
		make -C $$part install PREFIX="$(PREFIX)"; \
	done
	@mkdir -p $(PREFIX)/share/backgrounds/
	@cp -fv background.png $(PREFIX)/share/backgrounds/va11halla-background.png
	
uninstall:
	@for part in $(PARTS); do \
		make -C $$part uninstall PREFIX="$(PREFIX)"; \
	done
	@rm -v $(PREFIX)/share/backgrounds/va11halla-background.png

