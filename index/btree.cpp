#include "btree.h"

/*-------- BNode--------*/

BNode::BNode() {
  this->m_id = -1;
  this->numkeys = 0;
  this->leaf = 1;
}
BNode::~BNode() {
}
int BNode::id() {
  return m_id;
}
void BNode::init(int nid) {
  this->m_id = nid;
  this->numkeys = 0;
  this->leaf = 1;
  this->next[MAX_DEGREE+1] = 0x53535353;
}

/*-------- BManager--------*/

BManager::BManager() {
  this->num_nodes = 0;
  this->root_node_id = -1;   // should be fetched from file
  memset(bitmap, 0, sizeof(bitmap[0]) * MEMORY_BUFF);
  memset(pool, -1, sizeof(pool[0]) * MEMORY_BUFF);
}
BManager::~BManager() {
  meta_file = fopen(meta_path.c_str(), "w");
  fprintf(meta_file, "%d\n", num_nodes);
  fprintf(meta_file, "%d\n", root_node_id);
  fclose(meta_file);
  // flush all changed, or new pages to disk
  map<int, int>::iterator it;
  for (it = nodemap.begin(); it != nodemap.end(); ++it) {
    int nodeid = it->first;
    int pageid = it->second;
    if (bitmap[pageid] == 3 && pool[pageid].id() == nodeid) {
      flush(nodeid);
    }
  }
  fclose(data_file);
}


void BManager::init(string &meta_path, string &data_path) {
  this->meta_path = meta_path;
  this->data_path = data_path;

  meta_file = fopen(meta_path.c_str(), "r");
  if (meta_file != NULL) {
    fscanf(meta_file, "%d", &num_nodes);
    fscanf(meta_file, "%d", &root_node_id);
    fclose(meta_file);
  } else {
    data_file = fopen(data_path.c_str(), "w");
    fclose(data_file);
  }
  data_file = fopen(data_path.c_str(), "rb+");
}

void BManager::dump() {
  cout << endl;
  for (int i=0; i<MEMORY_BUFF; i++) {
    cout << bitmap[i] << " ";
  }
  cout << endl;
  for (int i=0; i<MEMORY_BUFF; i++) {
    cout << pool[i].id() << " ";
  }
  cout << endl;
}

BNode* BManager::new_node() {
  int pageid = allocate();
  int nodeid = num_nodes++;
  if (pageid < 0) {
    cout << "fail to allocate " << nodeid << endl;
    dump();
  }
  assert(pageid >= 0);

  nodemap[nodeid] = pageid;
  pool[pageid].init(nodeid);
  flush(nodeid);

  return &pool[pageid];
}

BNode* BManager::new_root() {
  BNode* root;
  root = new_node();
  root_node_id = root->id();
  return root;
}

BNode* BManager::get_root() {
  if (num_nodes == 0) {
    return new_root();
  } else {
    return get_node(root_node_id);
  }
}
BNode* BManager::get_node(int id) {
  int pageid;
  if (nodemap.find(id) == nodemap.end() || pool[nodemap[id]].id() != id) {
    pageid = allocate();
    if (pageid < 0) {
      cout << "fail to get " << id << endl; 
      dump();
    }
    assert(pageid >= 0);
    nodemap[id] = pageid;
    load(id);
  } else {
    pageid = nodemap[id];
    if (bitmap[pageid] != 3)  // dirty page is always dirty
      bitmap[pageid] = 1;
  }
  return &pool[pageid];
}
// Mark current node as 'soft free', maybe
// reused iBNode::f the id is not changed.
// Never free root node even the caller 
// mistakenly returns it.
// Also, dirty pages are always not for re-schedule
void BManager::return_node(int id) {
  int pageid = nodemap[id];
  if (id != root_node_id) {
    if (bitmap[pageid] == 3) {
      flush(id);
      //cout << id << " flushed" << endl;
    }
    bitmap[pageid] = 2;
  }
}

// Should mark updated node as dirty,
// when manager is closed, or when the 
// node is scheduled out of memory, flush
// it to disk
void BManager::update_node(int id) {
  bitmap[nodemap[id]] = 3;
}
int BManager::allocate() {
  for (int pageid = 0; pageid < MEMORY_BUFF; ++pageid) {
    if (bitmap[pageid] == 0) {
      bitmap[pageid] = 1;
      return pageid;
    }
  }
  for (int pageid = 0; pageid < MEMORY_BUFF; ++pageid) {
    if (bitmap[pageid] == 2) {
      bitmap[pageid] = 1;
      return pageid;
    }
  }
  return -1;
}

int BManager::filepos(int id) {
  return NODE_SZ * id;
}
// file will gracefully extend when reaching a new max_id
void BManager::flush(int id) {
  fseek(data_file, filepos(id), SEEK_SET);
  fwrite((void*)&pool[nodemap[id]], NODE_SZ, 1, data_file);
}
void BManager::load(int id) {
  fseek(data_file, filepos(id), SEEK_SET);
  fread((void*)&pool[nodemap[id]], NODE_SZ, 1, data_file);
}


/*-------- BTree --------*/

BTree::BTree(string &metapath, string &datapath) {
  this->manager.init(metapath, datapath);
}
BTree::~BTree() {
}

// Split node as two usually happen 
// when we walk down the btree
BNode* BTree::split(BNode* node) {
  BNode* twin = manager.new_node();
  int half = BNode::HALF;
  node->numkeys = half;
  twin->numkeys = half;
  twin->leaf = node->leaf;
  memcpy(twin->keys, &(node->keys[half+1]), sizeof(node->keys[0]) * (half));
  memcpy(twin->next, &(node->next[half+1]), sizeof(node->next[0]) * (half+1));
  return twin;
}

// Insert key to the tree, 
// duplicate key will be omited
void BTree::insert(int key) {
  BNode* cur = walk(key); 
  int pos = bsearch(cur->keys, cur->numkeys, key);
  if(array_insert(cur->keys, cur->numkeys, key, pos) >= 0) {
    cur->numkeys++;
    update(cur->id());
  }
  free(cur->id());
}

// Walk down the btree and search for key.
// Every full node (with numkeys == MAX_DEGREE)
// will be splitted.
// Return appropriate node for further insertion
// Should always return a node.
//
BNode* BTree::walk(int key) {
  BNode *cur = NULL, *next = manager.get_root();
  while (true) {
    // full node will split
    if (next->numkeys == BNode::MAX_DEGREE) {
      BNode* twin = split(next);
      int midkey = next->keys[BNode::HALF];
      int left  = next->id();
      int right = twin->id();
      if (cur == NULL) {  // the root splits
        cur = manager.new_root();
        cur->leaf = 0;
      }
      // update father node 
      int pos = bsearch(cur->keys, cur->numkeys, midkey);
      array_insert(cur->keys, cur->numkeys, midkey, pos);
      array_insert(cur->next, cur->numkeys+1, left,  pos);
      array_insert(cur->next, cur->numkeys+1, right, pos+1);
      cur->numkeys++;

      update(left);
      update(right);
      update(cur->id());
      if (key > midkey) {
        next = twin;
        free(left);
      } else if (key < midkey) {
        free(right);
      } else {
        free(left); free(right);
        return cur;
      }
    }
    if (cur != NULL) {
      free(cur->id());
    }
    if (next->leaf) {
      return next;
    }
    int i = bsearch(next->keys, next->numkeys, key);
    if (i < next->numkeys && key == next->keys[i]) {
      return next;
    }
    cur = next;
    next = get(next->next[i]);
  }
}

// Read-only lookup,
// Return appropriate node for further check
// Return NULL when no node found
// NOTE: when key doesn't exist, will not check further
//
BNode* BTree::search(int key) {
  BNode* cur = manager.get_root();
  while (true) {
    int i = bsearch(cur->keys, cur->numkeys, key);
    if (i >= cur->numkeys && cur->leaf) {
      return NULL;
    }
    if (i < cur->numkeys && key == cur->keys[i]) {
      return cur;
    }
    int id = cur->next[i];
    free(cur->id());
    cur = get(id);
  }
}

BNode* BTree::get(int id) {
  return manager.get_node(id);
}
void BTree::free(int id) {
  manager.return_node(id);
}
void BTree::update(int id) {
  manager.update_node(id);
}

void BTree::dump(BNode *n) {
  if (n==NULL)
    return;
  if (n->id() == manager.get_root()->id())
    cout << "*";
  cout << "" << n->id()<< "[";
  for (int i = 0; i < n->numkeys; i=n->numkeys)
    cout << n->keys[i];
  for (int i = 1; i < n->numkeys; i++)
    cout << " " << n->keys[i];
  cout << "] ";
  if (!n->leaf) {
    cout << "( ";
    for (int i = 0; i < n->numkeys + 1; i++)
      cout<<n->next[i]<<" ";
    cout << ") ";
  }
  cout << endl;
}
void BTree::inorder(BNode *n) {
  if (n->id() == manager.get_root()->id())
    cout << "*";
  cout << "" << n->id() << "[";
  for (int i = 0; i < n->numkeys-1; i++)
    cout << n->keys[i] << " ";
  for (int i = n->numkeys-1; i >= 0  && i < n->numkeys; i++)
    cout << n->keys[i];
  cout << "] ";
  int tmp[BNode::MAX_DEGREE+2];
  int sz;
  if (!n->leaf) {
    sz = n->numkeys;
    memcpy(tmp, n->next, sizeof(int) * (BNode::MAX_DEGREE+2));
    free(n->id());
    cout << "( ";
    for (int i = 0; i < sz + 1; i++)
      inorder(get(tmp[i]));
    cout << ") ";
  } else {
    free(n->id());
  }
}

void BTree::preorder(BNode *n) {
  int tkeys[BNode::MAX_DEGREE+1];
  int tnext[BNode::MAX_DEGREE+2];
  int sz=n->numkeys;
  if (n->leaf) {
    for (int i = 0; i < sz; i++)
      cout << n->keys[i] << endl;
    free(n->id());
    return;
  }
  memcpy(tkeys, n->keys, sizeof(int) * (BNode::MAX_DEGREE+1));
  memcpy(tnext, n->next, sizeof(int) * (BNode::MAX_DEGREE+2));

  free(n->id());

  for (int i = 0; i < sz; i++) {
    preorder(get(tnext[i]));
    cout << tkeys[i] << endl;
  }
  preorder(get(tnext[sz]));
}
void BTree::inorder() {
  inorder(manager.get_root());
  cout << endl;
}
void BTree::preorder() {
  preorder(manager.get_root());
}
