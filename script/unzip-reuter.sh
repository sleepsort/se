#!/bin/sh
# 
# This script will try to unzip RCV1 dataset, 
# from directory 'reuter-zip' to 'reuter'
#
# directory tree:
#
#  working directory:
#
#  ├── data -> ../data
#  ├── lib
#  │   ├── libz
#  │   └── libzip
#  ├── obj
#  ├── script        <- location to *run* this script
#  ├── src
#  └── test
#
#  data
#  ├── reuter        <- unzip destination
#  │   ├── disk1
#  │   └── disk2
#  ├── reuter-zip    <- unzip source
#  │   ├── disk1
#  │   └── disk2
#  └── shakespeare
#
function alert() {
  echo "folder 'reuter' already exist! it might be dangerous to overwrite";
  exit 1;
}
function usage() {
  echo "the comment of this script will provide right directory structure";
  exit 1;
}

[ -d ../data/reuter-zip ] || usage
[ ! -d ../data/reuter/disk1 ] || alert
[ ! -d ../data/reuter/disk2 ] || alert

mkdir -p ../data/reuter/disk1
mkdir -p ../data/reuter/disk2

cd ../data/reuter-zip/disk1
find -name "[0-9]*.zip" -exec unzip -qn {} -d ../../reuter/disk1/ \;

cd ../disk2
find -name "[0-9]*.zip" -exec unzip -qn {} -d ../../reuter/disk2/ \;

cd ../../reuter/
num=`find -name "*.xml" | wc -l`
echo "extracted $num xml files"
