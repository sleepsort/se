#include <iostream>
#include <cassert>
#include "index/reader.h"
#include "search/suggester.h"
#include "util/string.h"
#include "util/file.h"
#include "template/btree.h"
using namespace std;

#define SIZE 5
class ArrayKey {
 public:
  char buf[SIZE];
  ArrayKey() {}
  ArrayKey(char (&p)[SIZE]) {
    memcpy(buf, &p, sizeof(buf));
  }
  ArrayKey(ArrayKey &a) {
    memcpy(buf, a.buf, sizeof(buf));
  }
  inline bool operator==(const ArrayKey& n){return !strcmp(buf,n.buf);}
  inline bool operator!=(const ArrayKey& n){return !operator==(n);} 
  inline bool operator< (const ArrayKey& n){return strcmp(buf,n.buf)<0;} 
  inline bool operator> (const ArrayKey& n){return strcmp(buf,n.buf)>0;} 
  inline bool operator<=(const ArrayKey& n){return !operator> (n);} 
  inline bool operator>=(const ArrayKey& n){return !operator< (n);}
};
ostream& operator << (ostream &out, ArrayKey &a) {
  out << a.buf;
  return out;
}


// stupid tests

void testEditDistance() {
    assert(levendistance("dog", "do") == 1);
    assert(levendistance("cat", "cart") == 1);
    assert(levendistance("cat", "cut") == 1);
    assert(levendistance("cat", "act") == 2);
}

void testSuggestion() {
    IndexReader ir("data/index/");
    Suggester sg(ir);
    vector<int> collect;
    string w = "protestatiions";
    sg.kgram(w, collect);
    sg.levenrank(w, collect);
    dump(collect);
    assert(collect.size() == 2);
}
void testCharBTree() {
  string metapath = "data/index/meta.dat.char";
  string nodepath = "data/index/node.dat.char";
  string datapath = "data/index/data.dat.char";
  BTree<char> tree(metapath, nodepath, datapath);
  for (int i = 0; i < 25; i++) {
    char c = i+'a';
    tree.insert(c);
    tree.inorder();
  }
}
void testLongBTree() {
  string metapath = "data/index/meta.dat.int";
  string nodepath = "data/index/node.dat.int";
  string datapath = "data/index/data.dat.int";
  BTree<long> tree(metapath, nodepath, datapath);
  //for (long i = 0; i < 25; i++) {
  for (long i = 0; i < 10000; i++) {
    tree.insert(i);
    //tree.inorder();
  }
  tree.preorder();
}
void testArrayBTree() {
  string metapath = "data/index/meta.dat.arr";
  string nodepath = "data/index/node.dat.arr";
  string datapath = "data/index/data.dat.arr";
  BTree<ArrayKey> tree(metapath, nodepath, datapath);
  for (int i = 0; i < 25; i++) {
    char s[SIZE], t[SIZE];
    for (int j=0; j<SIZE-1; j++) {
      s[j] = (i+j) % 26 + 'a';
      t[SIZE-j-2] = s[j];
    }
    s[SIZE-1] = '\0';
    t[SIZE-1] = '\0';
    ArrayKey key(s);
    tree.insert(key, t, SIZE);
    tree.inorder();
  }
  char s[SIZE] = "mnop";
  ArrayKey nkey(s);
  int dataid = tree.search_data(nkey);
  if (dataid >=0) {
    char *tmp;
    int len;
    tmp = (char*)tree.get_data(dataid, len);
    cout << tmp << endl;
    delete tmp;
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
int main(int argc, char **argv) {
    //testEditDistance();
    //testSuggestion();
    //testCharBTree();
    //testLongBTree();
    testArrayBTree();
    //testExtension();
    cout << "passed!" << endl;
    return 0;
}
