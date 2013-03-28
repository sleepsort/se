#include <iostream>
#include <cassert>
#include "index/reader.h"
#include "search/suggester.h"
#include "util/string.h"
#include "util/file.h"
#include "template/btree.h"
#include "template/vb.h"
using namespace std;

// stupid tests

void testEditDistance() {
    assert(levendistance("dog", "do") == 1);
    assert(levendistance("cat", "cart") == 1);
    assert(levendistance("cat", "cut") == 1);
    assert(levendistance("cat", "act") == 2);
}

void testCharBTree() {
  string metapath = "data/index/meta.dat.char";
  string datapath = "data/index/data.dat.char";
  BTree<char> tree(metapath, datapath);
  for (int i = 0; i < 25; i++) {
    char c = i+'a';
    tree.insert(c);
    tree.inorder();
  }
}
void testLongBTree() {
  string metapath = "data/index/meta.dat.int";
  string datapath = "data/index/data.dat.int";
  BTree<long> tree(metapath, datapath);
  for (long i = 0; i < 25; i++) {
    tree.insert(i);
    tree.inorder();
  }
}
void testExtension() {
  assert(extension("hello.h") == "h");
  assert(extension("hello.h.cpp") == "cpp");
  assert(extension(".hidden.tmp") == "tmp");
  assert(extension(".hidden") == "hidden");
  assert(extension("hidden") == "");
  assert(extension("/.another") == "another");
}
void testVB() {
  unsigned raw_int[] = {2,5,6,7,9,11,12,34,0xfffffeff,0xffffffff};
  unsigned new_int[100];
  char buf[10000];
  int num, size;
  num = sizeof(raw_int) / sizeof(unsigned);
  encode_vb(raw_int, num, buf, size);
  decode_vb(buf, size, new_int, num);
  for (int i = 0; i< num; i++)
    assert(new_int[i] == raw_int[i]);

  unsigned long long raw_long[] = {2,5,6,7,9,11,12,34,0xfffffffffffffeffL, 0xffffffffffffffffL};
  unsigned long long new_long[100];
  num = sizeof(raw_long) / sizeof(unsigned long long);
  encode_vb(raw_long, num, buf, size);
  decode_vb(buf, size, new_long, num);
  for (int i = 0; i< num; i++)
    assert(new_long[i] == raw_long[i]);
}
int main(int argc, char **argv) {
    testEditDistance();
    //testCharBTree();
    //testLongBTree();
    testExtension();
    testVB();
    cout << "passed!" << endl;
    return 0;
}
