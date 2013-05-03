#!/usr/bin/env vim
V  := @
RM += -r

all: always
	$(V)$(MAKE) --no-print-directory -C src/ -f Makefile all

test:
	@./obj/test

sch:
	@./obj/search data/index

rnk:
	@./obj/score data/index

idx-small:
	$(V)$(RM) data/index
	@./obj/index data/shakespeare shakes data/index

idx-test-medium:
	$(V)$(RM) data/index
	@./obj/index data/reuter-small rcv1 data/index

idx-medium:
	$(V)$(RM) data/index
	@./obj/index data/reuter rcv1 data/index

idx-test-large:
	$(V)$(RM) data/index
	@./obj/index data/gov2-mini gov2 data/index

idx-large:
	$(V)$(RM) data/index
	@./obj/index data/gov2 gov2 data/index

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
