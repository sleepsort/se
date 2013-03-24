#!/usr/bin/env vim
V  := @
RM += -r

all:
	$(V)$(MAKE) --no-print-directory -C src/ -f Makefile all

test:
	@./obj/test

sch:
	@./obj/search data/index

idx-small:
	$(V)$(RM) data/index
	@./obj/index data/shakespeare data/index

idx-medium:
	$(V)$(RM) data/index
	@./obj/index data/reuter data/index

xml:
	@./obj/parse data/reuter/disk1/97950newsML.xml


.PHONY:clean always reset test
clean:
	$(V)$(RM) obj 2>/dev/null
reset:
	$(V)$(RM) data/index
shared:
	@LD_LIBRARY_PATH=./lib/libzip:$(LD_LIBRARY_PATH) ./obj/xxx
