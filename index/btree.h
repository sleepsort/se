#ifndef BTREE_H_
#define BTREE_H_

#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <string>
#include <iostream>
#include <map>
using namespace std;

// TODO(lcc): nodes might split two times
// TODO(billy): userspace chunk should be byte aligned.
// TODO(billy): check before really insert, in case it explodes
// TODO(billy): find a clear way to merge offset & size, forexample only offset
//              (with end offset) should be ok!
// TODO(billy): maximum size of BKey should not exceed UPPERBOUND / 2 !

// In-memory and in-disk format of b-tree node
// NOTE: this structure should always be 
// byte-aligned

class BKey {
 public: 
  BKey(int x) {
  }
  BKey(void* bytes, int len) {
  }
  void* bytes() {
    return m_bytes;
  }
  int len() {
    return m_len;
  }
  int compare(BKey &another) {
    return 0;
  }
 private:
  void *m_bytes;
  int m_len;
};

class BNode {
 public:
  static const int LOWER_BOUND = 3;
  static const int UPPER_BOUND = LOWER_BOUND * 2 - 1;
  static const int HALF = UPPER_BOUND / 2;
  BNode();
  ~BNode();
 
 public:
  int id();
  void init(int nid);

  int find(BKey& key);
  int insert(BKey& key, int pos);
  int midpos();
  int end();

 public:
  char field[UPPER_BOUND];       // key field, one key might consist of several bytes
  int offset[UPPER_BOUND + 1];   // offset of each field, in bytes
  int length[UPPER_BOUND + 1];   // length of each field, in bytes
  int next  [UPPER_BOUND + 2];   // next ids
  int numkeys;                   // num of keys, or num of next ids-1
  int leaf;

 private:
  int m_id;
};


// Memory scheduler, maintaining a memory pool for b-tree nodes, 
// every time the caller gets one page, it is responsible to return 
// the page back to manager.
// every time the caller modifies one page, it should also tell
// manager to update it.
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
    void dump();

  private:
    int allocate();
    int filepos(int nodeid);
    void flush(int nodeid);
    void load(int nodeid);

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

// The tree only maintains the root node, other nodes are
// requested from BManager
class BTree {
 public:
  BTree(string &metapath, string &datapath);
  ~BTree();
  void insert(BKey& key);
  BNode* walk(BKey& key);
  BNode* search(BKey& key);

 private:
  BNode* split(BNode* cur);

 private:
  BManager manager;
};

#endif  // BTREE_H_
