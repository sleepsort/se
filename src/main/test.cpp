#include <iostream>
#include <cassert>
#include <ctime>
#include "util/string.h"
#include "util/file.h"
#include "util/head.h"
#include "index/reader.h"
#include "search/suggester.h"
#include "search/permutree.h"
#include "template/btree.h"
#include "template/compress.h"
using namespace std;

time_t seed = time(0);

ostream& operator << (ostream &out, Permuterm &p) {
  out << p.buf;
  return out;
}

class Random {
  vector<int> pool;
  bool exclude;
 public:
  Random(int size, bool type) {
    exclude = type;
    for (int i = 0; i < size; ++i)
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

void testCharBTree() {
  Random ran(26, true);
  string prefix = "data/index/char";
  BTree<char> tree(prefix);
  //for (int i = 0; i <= 25; ++i) {
  for (int i = 25; i >= 0; i--) {
    char c = ran.next() + 'a';
    tree.insert(c);
  }
  tree.inorder();
}
void testLongBTree() {
  Random ran(10000, true);
  string prefix = "data/index/long";
  BTree<long long> tree(prefix);
  //for (long long i = 0; i < 25; ++i) {
  for (long long i = 0; i < 10000; ++i) {
    long long n = ran.next();
    tree.insert(n);
  }
  //tree.preorder();
  //tree.inorder();
}
void testArrayBTree() {
  string prefix = "data/index/arr";
  BTree<Permuterm> tree(prefix);
  char s[PERMU_BUF+1] = {0}, t[PERMU_BUF+1] = {0};
  for (int k = 0; k <= 25; k++) {
    for (int j = 0; j < PERMU_BUF; ++j) {
      s[j] = rand() % 26 + 'a';
      t[PERMU_BUF-j-1] = s[j];
    }
    Permuterm key(string(s), 0);
    tree.insert(key, t, PERMU_BUF+1);
  }
  tree.inorder();

  Permuterm nkey(string(s), 0);
  int dataid = tree.search_data(nkey), len;
  char *tmp;
  assert(dataid >= 0);
  tmp = (char*)tree.get_data(dataid, len);
  for (int i = 0; i < PERMU_BUF; ++i) {
    assert(tmp[PERMU_BUF - i - 1] == s[i]);
  }
  delete []tmp;
}

void testRange() {
  Random ran(26, false);
  string prefix = "data/index/arr";
  BTree<Permuterm> tree(prefix);
  char s[PERMU_BUF+1] = {0}, t[PERMU_BUF+1] = {0};
  for (int k = 0; k <= 70; k++) {
    for (int j = 0; j < PERMU_BUF; ++j) {
      s[j] = ran.next() + 'a';
    }
    Permuterm key(string(s), 0);
    tree.insert(key);
  }
  //tree.inorder();
  for (int j = 0; j < PERMU_BUF; ++j) {
    s[j] = ran.next() + 'a';
    t[j] = ran.next() + 'a';
  }
  Permuterm akey(string(s), 0), bkey(string(t), 0), ckey;
  pair<int, int> node, pos;
  if (akey > bkey) {
    ckey = akey;
    akey = bkey;
    bkey = ckey;
  }
  tree.search_key_between(akey, bkey, node, pos);

  vector<Permuterm> result, truth;
  int lid = node.first, rid = node.second;
  int lpos = pos.first, rpos = pos.second;
  assert(rid >= 0);

  while (lid != rid) {
    BNode<Permuterm> &n = tree.get_node(lid);
    for (int i = lpos; i < n.numkeys; i++) {
      result.push_back(n.keys[i]);
    }
    lid = n.sibling;
    tree.return_node(n.id);
    lpos = 0;
  }
  if (rid >= 0) {
    BNode<Permuterm> &n = tree.get_node(rid);
    for (int i = lpos; i < rpos; i++) {
      result.push_back(n.keys[i]);
    }
    tree.return_node(n.id);
  }

  int cur_id;
  cur_id = tree.search_node(akey);
  while (cur_id >= 0) {
    BNode<Permuterm>& n = tree.get_node(cur_id);
    for (int i = 0; i < n.numkeys; i++) {
      if (n.keys[i] >= akey && n.keys[i] < bkey) {
        truth.push_back(n.keys[i]);
      }
    }
    cur_id = n.sibling;
    tree.return_node(n.id);
  }
  assert(truth.size() == result.size());
  for (unsigned i = 0; i < result.size(); i++) {
    assert(truth[i]  == result[i]);
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
  for (int i = 0; i< num; ++i)
    assert(new_int[i] == raw_int[i]);

  unsigned long long raw_long[] = {2,5,6,7,9,11,12,34,0xfffffffffffffeffL, 0xffffffffffffffffL};
  unsigned long long new_long[100];
  num = sizeof(raw_long) / sizeof(unsigned long long);
  encode_vb(raw_long, num, buf, size);
  decode_vb(buf, size, new_long, num);
  for (int i = 0; i< num; ++i)
    assert(new_long[i] == raw_long[i]);
}
int main(int argc, char **argv) {
    srand(seed);
    cout << "seed="<<seed <<endl;
    //testEditDistance();
    //testSuggestion();
    //testCharBTree();
    //testLongBTree();
    //testArrayBTree();
    testRange();
    //testExtension();
    //testVB();
    cout << "passed!" << endl;
    return 0;
}
