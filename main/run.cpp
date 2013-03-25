#include <iostream>
#include "template/btree.h"
using namespace std;
void lookfor(BTree<int>& tree, int key) {
  BNode<int>* node;
  if ((node = tree.search(key))!=NULL) {
    cout << node->id() << "[" << key <<"]" << endl;
    tree.free(node->id());
  } else {
    cout << -1 << "[" << key <<"]" << endl;
  }
}
int main(int argc, char **argv) {
  string metapath = "data/meta.dat";
  string datapath = "data/data.dat";
  BTree<int> tree(metapath, datapath);
  //tree.inorder(tree.get(0));
  tree.inorder();
  //for (int i = 10000; i >= 0; i--) {
  //for (int i = 0; i <= 21; i++) {
  //for (int i = 0; i <= 10000; i++) {
  //for (int i = 10000; i >= 0; i--) {
  //for (int i = 1000; i >= 0; i--) {
  for (int i = 21; i >= 0; i--) {
    cout <<  "---------------" << i << endl;
    tree.insert(i);
    tree.inorder();
  }
  //tree.inorder();
  /*
  lookfor(tree, 3);
  lookfor(tree, 1);
  lookfor(tree, 16);
  lookfor(tree, 19);
  */
  /*
  int a[10] = {1,3,5,7,9,10,12,14,16,18};
  int b[21] = {0,0,1,1,2,2,3,3,4,4,5,6,6,7,7,8,8,9,9,10,10};
  for (int i = 0; i < 20; i++) {
    //cout << i << " " << bsearch(a,10,i) << endl;
    if (b[i] != bsearch(a,10,i)) {
     cout << i << " " << b[i] << " "<< bsearch(a,10,i) << endl;
    }
  }*/

  return 0;
}
