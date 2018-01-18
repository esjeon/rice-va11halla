
PREFIX = $(HOME)/.local/
CC = gcc

PARTS = dwm dmenu

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
	
uninstall:
	@for part in $(PARTS); do \
		make -C $$part uninstall PREFIX="$(PREFIX)"; \
	done

