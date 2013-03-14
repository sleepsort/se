#!/usr/bin/env vim
V  := @
RM += -r

all:
	$(V)make --no-print-directory -C src/ -f Makefile all

test:
	@./obj/test
sch:
	@./obj/search data/index
idx:
	$(V)$(RM) data/index
	@./obj/index data/shakespeare data/index

.PHONY:clean always reset test
clean:
	$(V)$(RM) obj 2>/dev/null
