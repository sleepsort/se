#!/usr/bin/env vim
V  := @
RM += -r

all: always
	$(V)$(MAKE) --no-print-directory -C src/ -f Makefile all

test:
	@./obj/test

sch:
	@./obj/search data/index

idx-small:
	$(V)$(RM) data/index
	@./obj/index data/shakespeare data/index

idx-test:
	$(V)$(RM) data/index
	@./obj/index data/reuter-small data/index

idx-medium:
	$(V)$(RM) data/index
	@./obj/index data/reuter data/index

.PHONY:clean always reset test
always:
	$(V) [ -d data ] || echo 'no data file!'
	$(V) [ -d data ] && mkdir -p data/index
clean:
	$(V)$(RM) obj 2>/dev/null
reset:
	$(V)$(RM) data/index
shared:
	@LD_LIBRARY_PATH=./lib/libzip:$(LD_LIBRARY_PATH) ./obj/xxx
