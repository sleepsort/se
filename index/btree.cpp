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
  this->next[UPPER_BOUND+1] = 'S';
}
int BNode::find(BKey& key) {
  int f = 0, t = numkeys-1, m = 0, cmp = 0;
  while (f <= t) {
    m = (f+t)/2;
    BKey ano((void*)(field + offset[m]), length[m]);
    cmp = key.compare(ano);
    if (cmp == 0) {
      return m;
    } else if (cmp > 0) {
      f = m+1;
    } else if (cmp < 0) {
      t = m-1;
    }
  }
  if (m < numkeys && cmp > 0) {
    return m+1;
  } else {
    return m;
  }
}
int BNode::insert(BKey& key, int pos) {
  int len = key.len();
  if (pos < numkeys) {
    BKey cur((void*)(field + offset[pos]), length[pos]);
    if (key.compare(cur) == 0)
      return -1;
  }
  assert(numkeys == 0 || len + offset[numkeys-1] + length[numkeys-1] <= UPPER_BOUND);

  if (pos < numkeys) {
    memmove(field + offset[pos] + len, field + offset[pos], end() - offset[pos]);
  }
  memcpy(field + offset[pos], key.bytes(), key.len());
  return pos;
}
int BNode::midpos() {
  int half_sz = UPPER_BOUND / 2;
  int upto = numkeys / 2;
  while (upto >= 0 && offset[upto] < HALF) {
    upto--;
  }
  assert(upto >= 0); // at least one key left!
  return upto; 
}
int BNode::end() {
  if (numkeys > 0)
    return offset[numkeys-1] + length[numkeys-1];
  return 0;
}


/*-------- BManager--------*/
BManager::BManager() {
  this->num_nodes = 0;
  this->root_node_id = -1;
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
// reused if the id is not changed.
// Will never free root node even the caller 
// mistakenly returns it.
// Also, dirty pages are always not for re-schedule
void BManager::return_node(int id) {
  int pageid = nodemap[id];
  if (id != root_node_id) {
    if (bitmap[pageid] == 3) {
      flush(id);
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
BNode* BTree::split(BNode* n) {
  BNode* t = manager.new_node();
  int midpos = n->midpos();
  assert(midpos > 0);
  assert(midpos < n->numkeys - 1);

  t->numkeys = n->numkeys - 1 - midpos;
  n->numkeys = midpos;

  t->leaf = n->leaf;

  memcpy(t->field, &(n->field[offset[midpos+1]]), 
         sizeof(char)*(n->end()-offset[midpos+1]));
  memcpy(t->next,  &(n->next[midpos+1]), 
         sizeof(int)*(twin->numkeys));
  return t;
}

// Insert key to the tree, 
// duplicate key will be omited
void BTree::insert(BKey& key) {
  BNode* cur = walk(key); 
  int pos = cur->find(key);
  if(cur->insert(key, pos) >= 0) {
    cur->numkeys++;
    manager.update_node(cur->id());
  }
  manager.return_node(cur->id());
}

// Walk down the btree and search for key.
// Every full node (with numkeys == UPPER_BOUND)
// will be splitted.
// Return appropriate node for further insertion
// Should always return a node.
//

BNode* BTree::walk(BKey& key) {
  BNode *cur = NULL, *next = manager.get_root();
  while (true) {
    // full node will split
    if (next->endpos() + key->len() >= BNode::UPPER_BOUND) {
    }
  /*
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

      manager.update_node(left);
      manager.update_node(right);
      manager.update_node(cur->id());
      if (key > midkey) {
        next = twin;
        manager.return_node(left);
      } else if (key < midkey) {
        manager.return_node(right);
      } else {
        manager.return_node(left);
        manager.return_node(right);
        return cur;
      }
    }
    if (cur != NULL) {
      manager.return_node(cur->id());
    }
    if (next->leaf) {
      return next;
    }
    int i = bsearch(next->keys, next->numkeys, key);
    if (i < next->numkeys && key == next->keys[i]) {
      return next;
    }
    cur = next;
    next = manager.get_node(next->next[i]);
  }*/
  return NULL;
}

// Read-only lookup,
// Return appropriate node for further check
// Return NULL when no node found
// NOTE: when key doesn't exist, will not check further
//

BNode* BTree::search(BKey& key) {
  /*
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
    manager.return_node(cur->id());
    cur = manager.get_node(id);
  }*/
  return NULL;
}
