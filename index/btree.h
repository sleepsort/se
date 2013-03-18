#ifndef BTREE_H_
#define BTREE_H_

#include <assert.h>
#include <memory.h>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include "util/util.h"
using namespace std;

class BNode {
 public:
  static const int MIN_DEGREE = 3;
  static const int MAX_DEGREE = MIN_DEGREE * 2 - 1;
  static const int HALF = MAX_DEGREE / 2;
  BNode();
  ~BNode();
 
 public:
  int id();
  void set(int nid);
  void clear();

 public:
  int keys[MAX_DEGREE + 1];
  int next[MAX_DEGREE + 2];
  int numkeys;
  bool leaf;

 private:
  int my_id;
};

// what do we store in disk?
// Metadata:
//   max node id, used for manager to allocate new node id
//   root infomation
class BManager {
  public:
    static const int MEMORY_BUFF = 1024;
    BManager();
    ~BManager();
    void init(string& path);
    BNode* new_node();
    BNode* new_root();
    BNode* get_root();
    BNode* get_node(int nodeid);
    void update_node(int nodeid);
    void return_node(int nodeid);
    int try_allocate();
  private:
    fstream stream;
    int root_node_id;
    int max_node_id;
    map<int, int> nodemap;        // node id => page id
    BNode pool[MEMORY_BUFF];
    int bitmap[MEMORY_BUFF];
};

class BTree {
 public:
  BTree(string& path);
  ~BTree();
  void insert(int key);
  BNode* walk(int key);
  BNode* search(int key);
  BNode* split(BNode* cur);
  void dump();
  void dump(BNode*);
  void dumpN(BNode*);

 private:
  BManager manager;
  BNode *root;
};

#endif  // BTREE_H_
