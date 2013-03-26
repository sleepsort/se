#include <iostream>
#include "template/btree.h"
using namespace std;

string metapath = "data/meta.dat";
string datapath = "data/data.dat";

void testchar() {
  BTree<char> tree(metapath, datapath);
  tree.inorder();
  //for (int i = 25; i >= 0; i--) {
  for (int i = 0; i <= 25; i++) {
    char c = i+'a';
    tree.insert(c);
    tree.inorder();
  }
}
void testint() {
  BTree<int> tree(metapath, datapath);
  //for (int i = 10000; i >= 0; i--) {
  for (int i = 0; i <= 21; i++) {
  //for (int i = 0; i <= 10000; i++) {
  //for (int i = 10000; i >= 0; i--) {
    tree.insert(i);
    tree.inorder();
  }
}

int main(int argc, char **argv) {
  testchar(); 
  //testint(); 
  return 0;
}
