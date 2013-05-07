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

############# SETUP ##############
# compile
all: always
	$(V)$(MAKE) --no-print-directory -C src/ -f Makefile all
	$(V)echo $(ID)

# prepare trec data and scripts
init-trec:
	$(V)mkdir log
	tar xvzf trec/trec_eval_latest.tar.gz -C trec
	tar xvzf trec/trec04.tgz -C trec
	tar xvzf trec/trec05.tgz -C trec
	tar xvzf trec/trec06.tgz -C trec
	$(V)$(MAKE) --quiet --no-print-directory -C trec/trec_eval.8.1/ -f Makefile


############# BUILD ##############
# build index, shakespeare data set: 5.2M
idx-small:
	$(V)$(RM) data/index
	@./obj/index data/shakespeare shakes data/index

# build index, RCV1 data set: 3.8G
idx-medium:
	$(V)$(RM) data/index
	@./obj/index data/reuter rcv1 data/index

# build index, GOV2 data set: 145G
# !NOTE: remove index with caution!
idx-large:
	$(V)#$(RM) data/index
	@./obj/index data/gov2 gov2 data/index



############# RUN  ##############
# search, no ranking
sch:
	@./obj/search data/index

# search with snippet, no ranking
smr:
	@./obj/summarize data/index

# ranking with multiple methods, support OR query only
rnk:
	@./obj/score data/index

# evaluation with trec queries
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

# check results (will show lines including recall and map)
view-all:
	@head -5 -v ./log/*eval*


############# DEBUG ONLY ##############
# part of RCV1 data set, two zip file only: 16M
idx-test-medium:
	$(V)$(RM) data/index
	@./obj/index data/reuter-test rcv1 data/index

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

# standard example index_00034.txt: 1.5G 
idx-test-large-6:
	$(V)$(RM) data/index
	@./obj/index data/gov2-test-6 gov2 data/index


.PHONY:clean always reset test
always:
	$(V) [ -d data ] || echo 'no data file!'
	$(V) [ -d data ] && mkdir -p data/index

test:
	@./obj/test

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
