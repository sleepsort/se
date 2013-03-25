#include <iostream>
#include "template/btree.h"
using namespace std;

void lookfor(BTree<int>& tree, int key) {
  int node_id;
  if ((node_id = tree.search(key)) != -1) {
    cout << node_id << "[" << key <<"]" << endl;
  } else {
    cout << -1 << "[" << key <<"]" << endl;
  }
}
int main(int argc, char **argv) {
  string metapath = "data/meta.dat";
  string datapath = "data/data.dat";
  BTree<int> tree(metapath, datapath);
  tree.inorder();
  //for (int i = 10000; i >= 0; i--) {
  //for (int i = 0; i <= 21; i++) {
  //for (int i = 0; i <= 10000; i++) {
  //for (int i = 10000; i >= 0; i--) {
  //for (int i = 1000; i >= 0; i--) {
  for (int i = 21; i >= 0; i--) {
    //cout <<  "---------------" << i << endl;
    tree.insert(i);
    tree.inorder();
  }
  //tree.preorder();

  return 0;
}
