#!/bin/sh -x
cd ..
make -j2
make
#make idx-test-large-6
make sch < test/gov2-00034 | diff -r -u test/gov2-00034.gold - | less 
