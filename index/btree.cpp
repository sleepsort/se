#include "btree.h"

BNode::BNode() {
  this->my_id = -1;
  this->parent = -1;
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
  this->parent = -1;
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
  BShit[pageid].set(nodeid);
  return &BShit[pageid];
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
  if (BShit[pageid].id() != id) {
    cout << id << " is returned to disk..." << endl;
    // try 
    //pageid = try_allocate();
    //nodemap[id] = pageid;
    //fetch data from disk
    return NULL;
  } else {
    bitmap[pageid] = 1;
    return &BShit[pageid];
  }
}
void BManager::return_node(int id) {
  // remember never return root page
  // bitmap[pageid] = 2;
}
void BManager::update_node(int id) {
  // flush to disk
  // also bitmap[pageid] = 2;
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

BNode* BTree::split(BNode* node) {
  BNode* twin = manager.new_node();
  int half = BNode::MAX_DEGREE / 2;
  node->numkeys = half;
  twin->numkeys = half + 1;
  twin->parent = node->parent;
  twin->leaf = node->leaf;
  memcpy(twin->keys, &(node->keys[half+1]), sizeof(node->keys[0]) * (half+1));
  memcpy(twin->next, &(node->next[half+1]), sizeof(node->next[0]) * (half+2));
  return twin;
}

void BTree::insert(int key) {
  BNode* cur = search(key); 
  array_insert(cur->keys, cur->numkeys, key); // TODO:should not insert duplicate
  assert(cur->leaf == true);
  cur->numkeys++;

  while (cur->numkeys > BNode::MAX_DEGREE) {
    int half = BNode::MAX_DEGREE/2;
    int newkey = cur->keys[half];
    BNode* ano = split(cur); 
    int left  = cur->id();
    int right = ano->id();
    int pos;
    // update children
    if (!cur->leaf) {
      for (int i = 0; i < ano->numkeys + 1; ++i) {
        BNode* son = manager.get_node(ano->next[i]);
        son->parent = ano->id();
        manager.update_node(ano->next[i]);
      }
    }
    // update parent
    if (cur->parent == -1) {  // is already root, create a clean new root
      BNode* newroot = manager.new_root();
      newroot->leaf = false;
      cur->parent = newroot->id();
      ano->parent = newroot->id();
      root = newroot;
    }
    cur = manager.get_node(cur->parent);
    manager.update_node(left);
    manager.update_node(right);
    pos = array_insert(cur->keys, cur->numkeys, newkey);
    cur->next[pos] = left;
    cur->next[pos+1] = right;
    cur->numkeys++;
  }
}

BNode* BTree::search(int key) {
  BNode* cur = root;
  while (true) {
    if (cur->leaf) {
      return cur;
    }
    int i = 0;
    while (i < cur->numkeys && key > cur->keys[i]) {
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
  if (n->parent == -1)
    cout << "*";
  cout << "" << n->id() << "=>" << n->parent << "[";
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
  if (n->parent == -1)
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
