#include <iostream>
#include <cassert>
#include <ctime>
#include "index/reader.h"
#include "search/suggester.h"
#include "util/string.h"
#include "util/file.h"
#include "template/btree.h"
using namespace std;

time_t seed = time(0);

#define LEN 5


class ArrayKey {
 public:
  char buf[LEN+1];
  ArrayKey() {}
  ArrayKey(char (&p)[LEN+1]) {
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

class Random {
  vector<int> pool;
  bool exclude;
 public:
  Random(int size, bool type) {
    exclude = type;
    for (int i=0; i<size; i++)
      pool.push_back(i);
  }
  int next() {
    assert(pool.size() > 0);
    int pos = rand() % pool.size();
    int n = pool[pos];
    if (exclude) {
      pool.erase(pool.begin()+pos);
    }
    return n;
  }
};


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
  Random ran(26, true);
  string metapath = "data/index/meta.dat.char";
  string nodepath = "data/index/node.dat.char";
  string datapath = "data/index/data.dat.char";
  BTree<char> tree(metapath, nodepath, datapath);
  //for (int i = 0; i <= 25; i++) {
  for (int i = 25; i >= 0; i--) {
    char c = ran.next()+'a';
    tree.insert(c);
    tree.inorder();
  }
}
void testLongBTree() {
  Random ran(10000, true);
  string metapath = "data/index/meta.dat.int";
  string nodepath = "data/index/node.dat.int";
  string datapath = "data/index/data.dat.int";
  BTree<long long> tree(metapath, nodepath, datapath);
  //for (long long i = 0; i < 25; i++) {
  for (long long i = 0; i < 10000; i++) {
    long long n = ran.next();
    tree.insert(n);
  }
  tree.preorder();
  tree.inorder();
}
void testArrayBTree() {
  Random ran(26, true);
  string metapath = "data/index/meta.dat.arr";
  string nodepath = "data/index/node.dat.arr";
  string datapath = "data/index/data.dat.arr";
  BTree<ArrayKey> tree(metapath, nodepath, datapath);
  for (int k = 0; k <= 25; k++) {
    int i = ran.next();
    char s[LEN+1], t[LEN+1];
    for (int j=0; j<LEN; j++) {
      s[j] = (i+j) % 26 + 'a';
      t[LEN-j-1] = s[j];
    }
    s[LEN] = '\0';
    t[LEN] = '\0';
    ArrayKey key(s);
    tree.insert(key, t, LEN+1);
    tree.inorder();
  }
  char tot[] = "abcdefghijklmnopqrstuvwxyz", s[LEN+1] = {0};
  memcpy(s, tot + (rand() % (26-LEN)), sizeof(char)*LEN);
  ArrayKey nkey(s);
  int dataid = tree.search_data(nkey);
  if (dataid >=0) {
    char *tmp;
    int len;
    tmp = (char*)tree.get_data(dataid, len);
    for (int i = 0; i < LEN; i++) {
      assert(tmp[LEN - i - 1] == s[i]);
    }
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
    srand(seed);
    cout << "seed="<<seed <<endl;
    //testEditDistance();
    //testSuggestion();
    //testCharBTree();
    //testLongBTree();
    testArrayBTree();
    //testExtension();
    cout << "passed!" << endl;
    return 0;
}
