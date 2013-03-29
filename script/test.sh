#!/bin/sh -x
cd ..
make -j2
make
make idx-small
make sch < test/proximity | diff -r -u test/proximity.gold - | less 
make sch < test/boolean | diff -r -u test/boolean.gold - | less 
