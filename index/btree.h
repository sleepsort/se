#ifndef BTREE_H_
#define BTREE_H_

#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <string>
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
  int leaf;

 private:
  int my_id;
};

class BManager {
  public:
    static const int MEMORY_BUFF = 4;
    static const int NODE_SZ = sizeof(BNode);
    BManager();
    ~BManager();
    void init(string &meta_path, string &data_path);
    BNode* new_node();
    BNode* new_root();
    BNode* get_node(int nodeid);
    BNode* get_root();
    void update_node(int nodeid);
    void return_node(int nodeid);
  private:
    int allocate();
    int filepos(int nodeid);
    void flush(int nodeid);
    void load(int nodeid);
    void dump();

  private:
    string meta_path;
    string data_path;
    FILE* meta_file;
    FILE* data_file;
    int root_node_id;
    int num_nodes;
    map<int, int> nodemap;        // node id => page id
    BNode pool[MEMORY_BUFF];
    int bitmap[MEMORY_BUFF];
};

class BTree {
 public:
  BTree(string &metapath, string &datapath);
  ~BTree();
  void insert(int key);
  BNode* walk(int key);
  BNode* search(int key);
  BNode* get(int key);
  void free(int key);
  void update(int key);
  void dump();
  void dump(BNode*);
  void dumpN(BNode*);

 private:
  BNode* split(BNode* cur);

 private:
  BManager manager;
  BNode *root;
};

#endif  // BTREE_H_
