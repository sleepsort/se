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
  //BTree<int> tree(metapath, datapath);
  BTree<char> tree(metapath, datapath);
  tree.inorder();
  //for (int i = 10000; i >= 0; i--) {
  //for (int i = 0; i <= 21; i++) {
  //for (int i = 0; i <= 10000; i++) {
  //for (int i = 10000; i >= 0; i--) {
  //for (int i = 1000; i >= 0; i--) {
  for (int i = 25; i >= 0; i--) {
    //cout <<  "---------------" << i << endl;
    char c = i+'a';
    tree.insert(c);
    tree.inorder();
  }
  //tree.preorder();

  return 0;
}
