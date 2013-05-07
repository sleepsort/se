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

smr:
	@./obj/summarize data/index

eval-04:
	@cat trec/trec04/04topics.701-750 | ./obj/topic | ./obj/trec data/index > /tmp/res
	@./trec/trec_eval.8.1/trec_eval -a trec/trec04/04.qrels.12-Nov-04 /tmp/res

idx-small:
	$(V)$(RM) data/index
	@./obj/index data/shakespeare shakes data/index

idx-test-medium:
	$(V)$(RM) data/index
	@./obj/index data/reuter-test rcv1 data/index

idx-medium:
	$(V)$(RM) data/index
	@./obj/index data/reuter rcv1 data/index

# multiple mini files: < 1M
idx-test-large-0:
	$(V)$(RM) data/index
	@./obj/index data/gov2-test-0 gov2 data/index

# single normal file: ~ 1.2G
idx-test-large-1:
	$(V)$(RM) data/index
	@./obj/index data/gov2-test-1 gov2 data/index

# multiple normal file: ~ 3.2G
idx-test-large-2:
	$(V)$(RM) data/index
	@./obj/index data/gov2-test-2 gov2 data/index

# special single file: ~ 6G
idx-test-large-3:
	$(V)$(RM) data/index
	@./obj/index data/gov2-test-3 gov2 data/index

# binary single file: ~ 970M
idx-test-large-4:
	$(V)$(RM) data/index
	@./obj/index data/gov2-test-4 gov2 data/index

# binary single file: 1.1G 
idx-test-large-5:
	$(V)$(RM) data/index
	@./obj/index data/gov2-test-5 gov2 data/index

# standard example : 1.5G 
idx-test-large-6:
	$(V)$(RM) data/index
	@./obj/index data/gov2-test-6 gov2 data/index

idx-large:
	$(V)#$(RM) data/index
	@./obj/index data/gov2 gov2 data/index

init-trec:
	tar xvzf trec/trec_eval_latest.tar.gz -C trec
	tar xvzf trec/trec04.tgz -C trec
	tar xvzf trec/trec05.tgz -C trec
	tar xvzf trec/trec06.tgz -C trec
	$(V)$(MAKE) --quiet --no-print-directory -C trec/trec_eval.8.1/ -f Makefile

.PHONY:clean always reset test
always:
	$(V) [ -d data ] || echo 'no data file!'
	$(V) [ -d data ] && mkdir -p data/index

clean:
	$(V)$(RM) obj 2>/dev/null

clean-trec:
	$(V)$(RM) trec/trec_eval.8.1 2>/dev/null
	$(V)$(RM) trec/trec04 2>/dev/null
	$(V)$(RM) trec/trec05 2>/dev/null
	$(V)$(RM) trec/trec06 2>/dev/null

reset:
	$(V)$(RM) data/index
	$(V) [ -d data ] || echo 'no data file!'
	$(V) [ -d data ] && mkdir -p data/index

shared:
	@LD_LIBRARY_PATH=./lib/libzip:$(LD_LIBRARY_PATH) ./obj/xxx
