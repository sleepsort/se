#!/usr/bin/env vim
V  := @
RM += -r
ID = $(shell date +"%Y-%m-%d.%H:%M")
EVAL_EXEC = ./trec/trec_eval.8.1/trec_eval -a 
QRY_04 = trec/trec04/04topics.701-750
QRY_05 = trec/trec05/05.topics.751-800
QRY_06 = trec/trec06/06.topics.801-850
QRY_ALL = trec/trec06/06.topics.701-850
REL_04 = trec/trec04/04.qrels.12-Nov-04 
REL_05 = trec/trec05/05.adhoc_qrels
REL_06 = trec/trec06/qrels.all
REL_ALL = trec/trec06/qrels.all

all: always
	$(V)$(MAKE) --no-print-directory -C src/ -f Makefile all
	$(V)echo $(ID)

test:
	@./obj/test

sch:
	@./obj/search data/index

rnk:
	@./obj/score data/index

smr:
	@./obj/summarize data/index

eval-04:
	@cat $(QRY_04) | ./obj/topic | ./obj/trec data/index > ./log/04-res.$(ID)
	@$(EVAL_EXEC) $(REL_04) ./log/04-res.$(ID) > ./log/04-eval.$(ID)
	@head -5 -v ./log/04-eval.$(ID)

eval-05:
	@cat $(QRY_05) | ./obj/topic | ./obj/trec data/index > ./log/05-res.$(ID)
	@$(EVAL_EXEC) $(REL_05) ./log/05-res.$(ID) | tee > ./log/05-eval.$(ID)
	@head -5 -v ./log/05-eval.$(ID)

eval-06:
	@cat $(QRY_06) | ./obj/topic | ./obj/trec data/index > ./log/06-res.$(ID)
	@$(EVAL_EXEC) $(REL_06) ./log/06-res.$(ID) | tee > ./log/06-eval.$(ID)
	@head -5 -v ./log/06-eval.$(ID)

eval-all:
	@cat $(QRY_ALL) | ./obj/topic | ./obj/trec data/index > ./log/all-res.$(ID)
	@$(EVAL_EXEC) $(REL_ALL) ./log/all-res.$(ID) | tee > ./log/all-eval.$(ID)
	@head -5 -v ./log/all-eval.$(ID)

view-all:
	@head -5 -v ./log/*eval*

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
	$(V)mkdir log
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
