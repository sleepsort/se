#include <iostream>
#include "index/btree.h"
using namespace std;
int main(int argc, char **argv) {
  string path = "data/tmp";
  BTree tree(path);
  tree.dump();
  for (int i = 0; i <= 20; i++) {
    tree.insert(i);
    tree.dump();
  }
  tree.insert(20);
  tree.dump();
  return 0;
}
