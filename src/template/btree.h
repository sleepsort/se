#ifndef TEMPLATE_BTREE_H_
#define TEMPLATE_BTREE_H_

#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <string>
#include <iostream>
#include <map>
using namespace std;

#define BLOCK_SIZE 5
#define CHUNK_SIZE 5
#define HALF_SIZE  (CHUNK_SIZE/2)

// TODO(lcc): nodes might split two times, when key size is not limited
// TODO(billy): userspace chunk should be byte aligned ?

// In-memory and in-disk format of b-tree node
// NOTE: this structure should always be 
// byte-aligned

template<class T>
class BNode {
 public:
  BNode();
  ~BNode();
  void init(int nid);
  int findkey(T& key);
  int addkey(T& key, int pos);
  int addnext(int left, int right, int pos);  // for non-leaf
  int adddata(int dataid, int pos);           // for leaf
  int ascendpos();
 public:
  int leaf;
  int id;
  int numkeys;
  int sibling;               // -1: last sibling, else: next sibling
  int next[CHUNK_SIZE + 2];  // when leaf, refer to data id, otherwise node id
  T keys[CHUNK_SIZE + 1];
};


// Memory scheduler, maintaining a memory pool for b-tree nodes, 
// every time the caller gets one page, it is responsible to return 
// the page back to manager.
// every time the caller modifies one page, it should also tell
// manager to update it.
template<class T>
class BManager {
  public:
    BManager();
    ~BManager();
    void init(string &meta_path, string &node_path, string &data_path);
    BNode<T>& new_node();
    BNode<T>& new_root();
    BNode<T>& get_node(int nodeid);
    BNode<T>& get_root();
    void update_node(int nodeid);
    void return_node(int nodeid);

    int  new_data(void *data, int length);
    void* get_data(int dataid, int &length);

  private:
    int allocate();
    void flush(int nodeid);
    void load(int nodeid);
    void optimize_data();
    void dump();

    long long nodefp(int nodeid);
    long long datafp(int dataid);

  private:
    static const int MEMORY_BUFF = 4;
    static const int NODE_SZ = sizeof(BNode<T>);
    enum MASK {
      PAGE_NULL    = 0x00,
      PAGE_LOCK    = 0x01,
      PAGE_DIRTY   = 0x02,
    };
    string meta_path;
    string node_path;
    string data_path;
    FILE* meta_file;
    FILE* node_file;
    FILE* data_file;

    int root_node_id;
    int num_nodes;
    int num_data;
    int optimized;
    vector<pair<long long, int> > data_field;

    map<int, int> nodemap;        // node id => page id
    BNode<T> pool[MEMORY_BUFF];
    int bitmap[MEMORY_BUFF];
};

// The tree only maintains the root node, other nodes are
// requested from BManager
template<class T>
class BTree {
 public:
  BTree(string &metapath, string &nodepath, string &datapath);
  ~BTree();
  void insert(T& key, void *data, int length);
  int search_node(T& key);
  int search_data(T& key);

  void* get_data(int dataid, int &length);

  void inorder();
  void inorder(BNode<T>&);
  void preorder();
  void preorder(BNode<T>&);

 private:
  void split(int p_id, int n_id);
  void insert(int p_id, int n_id, T& key, void *data, int length);
  int search(T& key, bool force);

  BNode<T>& get_node(int nodeid);
  void return_node(int nodeid);
  void update_node(int nodeid);

  void dump(BNode<T>&);

 private:
  BManager<T> manager;
};

#include "template/btree.cpp"
#endif  // TEMPLATE_BTREE_H_
