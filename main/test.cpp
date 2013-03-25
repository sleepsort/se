#include <iostream>
#include "index/btree.h"
using namespace std;
int main(int argc, char **argv) {
  int n, id;
  string metapath = "data/meta.dat";
  string datapath = "data/data.dat";
  BTree<int> tree(metapath, datapath);
  cin>>n;
  while(n--) {
    cin >> id;
    tree.insert(id);
  }
  //tree.preorder();
  tree.inorder();
  return 0;
}
