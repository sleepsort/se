#include <iostream>
#include "index/btree.h"
using namespace std;
int main(int argc, char **argv) {
  string path = "data/tmp";
  BTree tree(path);
  tree.dump();
  for (int i = 0; i < 21; i++) {
    cout << "--------" << i << endl;
    tree.insert(i);
    tree.dump();
  }
  return 0;
}
