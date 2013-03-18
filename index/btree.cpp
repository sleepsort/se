#include "btree.h"

BNode::BNode() {
  this->my_id = -1;
  this->numkeys = 0;
  this->leaf = true;
}
BNode::~BNode() {
}
int BNode::id() {
  return my_id;
}
void BNode::set(int nid) {
  this->my_id = nid;
}
void BNode::clear() {
  this->numkeys = 0;
  this->leaf = true;
  this->next[0] = -1;
}

BManager::BManager() {
  this->max_node_id = 0;
  this->root_node_id = -1;   // should be fetched from file
  memset(bitmap, 0, sizeof(bitmap[0]) * MEMORY_BUFF);
}
BManager::~BManager() {
  if (stream.is_open())
    stream.close();
}
void BManager::init(string& path) {
  stream.open(path.c_str(), ios::in | ios::out | ios::binary);
}

BNode* BManager::new_node() {
  int pageid = try_allocate();
  int nodeid = max_node_id++;
//  cout << "newnode: nodeid=" << nodeid<< " pageid="<<pageid<<endl;
  if (pageid == -1) {
    return NULL;  // should never happen !
  }
  nodemap[nodeid] = pageid;
  pool[pageid].set(nodeid);
  return &pool[pageid];
}

BNode* BManager::new_root() {
  BNode* root;
  root = new_node();
  root->clear();
  root_node_id = root->id();
  return root;
}

BNode* BManager::get_root() {
  if (max_node_id == 0) {
    return new_root();
  } else {
    return get_node(root_node_id);
  }
}
BNode* BManager::get_node(int id) {
  int pageid = nodemap[id];
  if (pool[pageid].id() != id) {
    cout << id << " is returned to disk..." << endl;
    // try 
    //pageid = try_allocate();
    //nodemap[id] = pageid;
    //fetch data from disk
    return NULL;
  } else {
    bitmap[pageid] = 1;
    return &pool[pageid];
  }
}
// Mark current node as 'soft free', maybe
// reused if the id is not changed.
// Never free root node even the caller 
// mistakenly returns it.
void BManager::return_node(int id) {
  if (id != root_node_id)
    bitmap[nodemap[id]] = 2;
}

// Should mark updated node as dirty,
// when manager is closed, or when the 
// node is scheduled out of memory, flush
// it to disk
void BManager::update_node(int id) {
  // flush to disk
  // also bitmap[pageid] = 3;
}
int BManager::try_allocate() {
  for (int pageid = 0; pageid < MEMORY_BUFF; ++pageid) {
    if (!bitmap[pageid]) {
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


BTree::BTree(string &path) {
  this->manager.init(path);
  this->root = manager.get_root();
}
BTree::~BTree() {
}

// Split node when it reaches numkeys == MAX_DEGREE,
// and this usually happen when we walk down the btree
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

// Insertion without duplicate
void BTree::insert(int key) {
  BNode* cur = walk(key); 
  if (array_insert(cur->keys, cur->numkeys, key) != -1) {
    cur->numkeys++;
    manager.update_node(cur->id());
    manager.return_node(cur->id());
  }
}

// Walk down the btree and search for key.
// Every full node (with numkeys == MAX_DEGREE)
// will be splitted.
// Return appropriate node for further insertion
//
BNode* BTree::walk(int key) {
  BNode *cur = NULL, *next = root;
  while (true) {
    if (next->numkeys == BNode::MAX_DEGREE) {
      BNode* twin = split(next);
      int midkey = next->keys[BNode::HALF];
      int left  = next->id();
      int right = twin->id();
      if (cur == NULL) {  // the root splits
        cur = manager.new_root();
        cur->leaf = false;
        root = cur;
      }
      int pos = array_insert(cur->keys, cur->numkeys, midkey);
      cur->next[pos] = left;
      cur->next[pos + 1] = right;
      cur->numkeys++;
      manager.update_node(left);
      manager.update_node(right);
      manager.update_node(cur->id());
      if (key > midkey) {
        manager.return_node(left);
        next = twin;
      } else if (key < midkey) {
        manager.return_node(right);
      } else {
        manager.return_node(left);
        manager.return_node(right);
        return cur;
      }
    }
    if (next->leaf) {
      return next;
    }
    int i = 0;
    while (i < next->numkeys && key > next->keys[i]) {
      i++;
    }
    if (i < next->numkeys && key == next->keys[i]) {
      return next;
    }
    if (cur != NULL) {
      manager.return_node(cur->id());
    }
    cur = next;
    next = manager.get_node(next->next[i]);
  }
}

// Readonly lookup,
// Return appropriate node for further check
//
BNode* BTree::search(int key) {
  BNode* cur = root;
  while (true) {
    if (cur->leaf) {
      return cur;
    }
    int i = 0;
    while (i < cur->numkeys && key > cur->keys[i]) { // no binary search, do we?
      i++;
    }
    if (i < cur->numkeys && key == cur->keys[i]) {
      return cur;
    }
    cur = manager.get_node(cur->next[i]);
  }
}

void BTree::dumpN(BNode *n) {
  if (n==NULL)
    return;
  if (n->id() == root->id())
    cout << "*";
  cout << "" << n->id()<< "[";
  for (int i = 0; i < n->numkeys; i=n->numkeys) {
    cout << n->keys[i];
  }
  for (int i = 1; i < n->numkeys; i++) {
    cout << " " << n->keys[i];
  }
  cout << "] ";
  if (!n->leaf) {
    cout << "( ";
    for (int i = 0; i < n->numkeys + 1; i++) {
      cout<<n->next[i]<<" ";
    }
    cout << ") ";
  }
  cout << endl;
}
void BTree::dump(BNode *n) {
  if (n->id() == root->id())
    cout << "*";
  cout << "" << n->id() << "[";
  for (int i = 0; i < n->numkeys; i=n->numkeys) {
    cout << n->keys[i];
  }
  for (int i = 1; i < n->numkeys; i++) {
    cout << " " << n->keys[i];
  }
  cout << "] ";
  if (!n->leaf) {
    cout << "( ";
    for (int i = 0; i < n->numkeys + 1; i++) {
      dump(manager.get_node(n->next[i]));
    }
    cout << ") ";
  }
}
void BTree::dump() {
  dump(root);
  cout << endl;
}
