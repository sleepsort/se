#include <iostream>
#include "index/btree.h"
using namespace std;
int main(int argc, char **argv) {
  int n, id;
  string metapath = "data/meta.dat";
  string datapath = "data/data.dat";
  BTree tree(metapath, datapath);
  cin>>n;
  while(n--) {
    cin >> id;
    tree.insert(id);
  }
  //tree.sort();
  tree.dump();
  return 0;
}
