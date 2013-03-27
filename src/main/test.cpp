#include <iostream>
#include <cassert>
#include "index/reader.h"
#include "search/suggester.h"
#include "util/string.h"
#include "util/file.h"
#include "template/btree.h"
using namespace std;

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
  string datapath = "data/index/data.dat.char";
  BTree<char> tree(metapath, datapath);
  for (int i = 0; i < 25; i++) {
    char c = i+'a';
    tree.insert(c);
    tree.inorder();
  }
}
void testIntBTree() {
  string metapath = "data/index/meta.dat.int";
  string datapath = "data/index/data.dat.int";
  BTree<int> tree(metapath, datapath);
  for (int i = 0; i < 25; i++) {
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
int main(int argc, char **argv) {
    //testEditDistance();
    //testSuggestion();
    testCharBTree();
    //testIntBTree();
    //testExtension();
    cout << "passed!" << endl;
    return 0;
}
