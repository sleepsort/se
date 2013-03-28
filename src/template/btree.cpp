#include "template/btree.h"

/*-------- BNode--------*/

template<class T>
BNode<T>::BNode() {
  this->id = -1;
  this->numkeys = 0;
  this->sibling = 0;
}
template<class T>
BNode<T>::~BNode() {
}
template<class T>
void BNode<T>::init(int nid) {
  this->id = nid;
  this->numkeys = 0;
  this->sibling = 0;
  this->next[CHUNK_SIZE+1] = 0x53535353;
}
template<class T>
int BNode<T>::findkey(T& key) {
  int f = 0, t = numkeys -1, m = 0;
  int cmp = 0;
  while (f <= t) {
    m = (f+t)/2;
    cmp = keys[m] == key ? 0 : keys[m] < key ? -1 : 1;
    if (cmp == 0) {
      return m;
    } else if (cmp < 0) {
      f = m+1;
    } else if (cmp > 0) {
      t = m-1;
    }
  }
  if (m < numkeys && cmp < 0) {
    return m+1;
  } else {
    return m;
  }
}

template<class T>
int BNode<T>::addkey(T& key, int pos) {
  if (pos < numkeys && keys[pos] == key)
    return -1;
  int j = numkeys;
  while (j > pos) {
    keys[j] = keys[j-1];
    j--;
  }
  keys[pos] = key;
  return pos;
}

template<class T>
int BNode<T>::addnext(int left, int right, int pos) {
  assert(!leaf());
  assert(!(numkeys == 0 && pos > 0));
  if (numkeys == 0) {
    next[0] = left;
    next[1] = right;
    return 0;
  }
  int j = numkeys+1, n, p;
  if (next[pos] == left) {
    n = right;
    p = pos + 1;
  } else {
    n = left;
    p = pos;
  }
  while (j > p) {
    next[j] = next[j-1];
    j--;
  }
  next[p] = n;
  return pos;
}

template<class T>
int BNode<T>::adddata(int dataid, int pos) {
  assert(leaf());
  assert(pos >= 0 && pos <= numkeys);
  int j = numkeys;
  while (j > pos) {
    next[j] = next[j-1];
    j--;
  }
  next[pos] = dataid;
  return pos;
}

// the key in this position will be
// ascended to father node
template<class T>
int BNode<T>::ascendpos() {
  unsigned accum = 0, i = 0, sz = numkeys;
  for (i = 0; i < sz; i++) {
    accum += sizeof(T);
    if (accum > HALF_SIZE*sizeof(T)) {
      break;
    }
  }
  return i;
}
template<class T>
bool BNode<T>::leaf() {
  if (sibling < 0) {
    return false;
  }
  return true;
}



/*-------- BManager--------*/

template<class T>
BManager<T>::BManager() {
  this->num_nodes = 0;
  this->root_node_id = -1;   // should be fetched from file
  memset(bitmap, 0, sizeof(bitmap[0]) * MEMORY_BUFF);
  memset(pool, -1, sizeof(pool[0]) * MEMORY_BUFF);
}
template<class T>
BManager<T>::~BManager() {
  meta_file = fopen(meta_path.c_str(), "w");
  fprintf(meta_file, "%d\n", num_nodes);
  fprintf(meta_file, "%d\n", root_node_id);
  fclose(meta_file);
  // flush all changed, or new pages to disk
  map<int, int>::iterator it;
  for (it = nodemap.begin(); it != nodemap.end(); ++it) {
    int nodeid = it->first;
    int pageid = it->second;
    if (bitmap[pageid] == 3 && pool[pageid].id == nodeid) {
      flush(nodeid);
    }
  }
  fclose(data_file);
}


template<class T>
void BManager<T>::init(string &meta_path, string &data_path) {
  this->meta_path = meta_path;
  this->data_path = data_path;

  meta_file = fopen(meta_path.c_str(), "r");
  if (meta_file != NULL) {
    int r;
    if ((r=fscanf(meta_file, "%d", &num_nodes)) < 0) {
      fprintf(stderr,"error loading numnodes\n");
    }
    if ((r=fscanf(meta_file, "%d", &root_node_id)) < 0) {
      fprintf(stderr,"error loading root_id\n");
    }
    fclose(meta_file);
  } else {
    data_file = fopen(data_path.c_str(), "w");
    fclose(data_file);
  }
  data_file = fopen(data_path.c_str(), "rb+");
}

template<class T>
void BManager<T>::dump() {
  cout << endl;
  for (int i=0; i<MEMORY_BUFF; i++) {
    cout << bitmap[i] << " ";
  }
  cout << endl;
  for (int i=0; i<MEMORY_BUFF; i++) {
    cout << pool[i].id << " ";
  }
  cout << endl;
}

template<class T>
BNode<T>& BManager<T>::new_node() {
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

  return pool[pageid];
}

template<class T>
BNode<T>& BManager<T>::new_root() {
  BNode<T>& root = new_node();
  root_node_id = root.id;
  return root;
}

template<class T>
BNode<T>& BManager<T>::get_root() {
  if (num_nodes == 0) {
    return new_root();
  } else {
    return get_node(root_node_id);
  }
}
template<class T>
BNode<T>& BManager<T>::get_node(int id) {
  int pageid;
  if (nodemap.find(id) == nodemap.end() || pool[nodemap[id]].id != id) {
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
  return pool[pageid];
}
// Mark current node as 'soft free', maybe
// reused iBNode::f the id is not changed.
// Never free root node even the caller 
// mistakenly returns it.
// Also, dirty pages are always not for re-schedule
template<class T>
void BManager<T>::return_node(int id) {
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
template<class T>
void BManager<T>::update_node(int id) {
  bitmap[nodemap[id]] = 3;
}
template<class T>
int BManager<T>::allocate() {
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

template<class T>
int BManager<T>::filepos(int id) {
  return NODE_SZ * id;
}
// file will gracefully extend when reaching a new max_id
template<class T>
void BManager<T>::flush(int id) {
  fseek(data_file, filepos(id), SEEK_SET);
  fwrite((void*)&pool[nodemap[id]], NODE_SZ, 1, data_file);
}
template<class T>
void BManager<T>::load(int id) {
  int r;
  fseek(data_file, filepos(id), SEEK_SET);
  if ((r=fread((void*)&pool[nodemap[id]], NODE_SZ, 1, data_file)) <= 0) {
    fprintf(stderr, "error loading node: %d\n", id);
  }
}


/*-------- BTree --------*/

template<class T>
BTree<T>::BTree(string &metapath, string &datapath) {
  this->manager.init(metapath, datapath);
}
template<class T>
BTree<T>::~BTree() {
}

// Split node as two usually happen 
// when we walk down the btree
template<class T>
void BTree<T>::split(int p_id, int n_id) {
  BNode<T>& n = get(n_id);
  BNode<T>& t = manager.new_node();
  int pos = n.ascendpos();
  T& newkey = n.keys[pos];
  t.sibling = n.sibling;
  t.numkeys = n.numkeys - pos - 1;
  n.numkeys = pos;
  memcpy(t.keys, &(n.keys[pos+1]), sizeof(T)*(t.numkeys));
  if (!n.leaf()) {
    memcpy(t.next, &(n.next[pos+1]), sizeof(int)*(t.numkeys+1));
  } else {
    n.sibling = t.id; 
    memcpy(t.next, &(n.next[pos+1]), sizeof(int)*(t.numkeys));
  }
  if (p_id == -1) {  // root splits
    BNode<T>& pp = manager.new_root();
    pp.sibling = -1;
    p_id = pp.id;
    free(pp.id);
  }
  BNode<T>& p = get(p_id);
  int newpos = p.findkey(newkey);
  p.addkey(newkey, newpos);
  p.addnext(n.id, t.id, newpos);
  p.numkeys++;
  update(n.id); free(n.id);
  update(t.id); free(t.id);
  update(p.id); free(p.id);
  return;
}


// Insert key to the tree, 
// duplicate key will be omited
template<class T>
void BTree<T>::insert(T& key, void *data=NULL, int length=0) {
  int rootid = manager.get_root().id;
  free(rootid);
  insert(-1, rootid, key, data, length);
}

template<class T>
void BTree<T>::insert(int p_id, int n_id, T& key, void *data, int length) {
  BNode<T> &n = get(n_id);
  int pos = n.findkey(key), sz = n.numkeys;
  if (pos < sz && key == n.keys[pos]) { // duplicated
    return;
  }
  if (n.leaf() && n.addkey(key, pos) >= 0) {
    n.adddata(-1, pos);
    n.numkeys++;
    update(n_id);
    free(n_id);
  } else {
    int m_id = n.next[pos];
    free(n_id);
    insert(n_id, m_id, key, data, length);
  }
  BNode<T> &nn = get(n_id);
  if (nn.numkeys >= CHUNK_SIZE) {
    free(n_id);
    split(p_id, n_id);
  } else {
    free(n_id);
  }
}

// Read-only lookup,
// Return appropriate node for further check
// Return NULL when no node found
// NOTE: when key doesn't exist, will not check further
//
template<class T>
int BTree<T>::search(T& key, bool force=false) {
  int cur_id = manager.get_root().id;
  while (true) {
    BNode<T>& cur = manager.get_node(cur_id);
    int i = cur.findkey(key);
    if (i >= cur.numkeys && cur.leaf()) {
      return force ? cur_id : -1;
    }
    if (i < cur.numkeys && key == cur.keys[i]) {
      free(cur_id);
      return cur_id;
    }
    cur_id = cur.next[i];
    free(cur.id);
  }
}

template<class T>
BNode<T>& BTree<T>::get(int id) {
  return manager.get_node(id);
}
template<class T>
void BTree<T>::free(int id) {
  manager.return_node(id);
}
template<class T>
void BTree<T>::update(int id) {
  manager.update_node(id);
}

template<class T>
void BTree<T>::dump(BNode<T>& n) {
  if (n==NULL)
    return;
  if (n.id == manager.get_root().id)
    cout << "*";
  cout << "" << n.id<< "[";
  for (int i = 0; i < n.numkeys; i=n.numkeys)
    cout << n.keys[i];
  for (int i = 1; i < n.numkeys; i++)
    cout << " " << n.keys[i];
  cout << "] ";
  if (!n.leaf()) {
    cout << "( ";
    for (int i = 0; i < n.numkeys + 1; i++)
      cout << n.next[i] << " ";
    cout << ") ";
  }
  cout << endl;
}

template<class T>
void BTree<T>::inorder(BNode<T>& n) {
  if (n.id == manager.get_root().id)
    cout << "*";
  cout << "" << n.id << "[";
  for (int i = 0; i < n.numkeys-1; i++)
    cout << n.keys[i] << " ";
  for (int i = n.numkeys-1; i >= 0  && i < n.numkeys; i++)
    cout << n.keys[i];
  cout << "] ";
  int tmp[CHUNK_SIZE+2];
  int sz;
  if (!n.leaf()) {
    sz = n.numkeys;
    memcpy(tmp, n.next, sizeof(int) * (CHUNK_SIZE+2));
    free(n.id);
    cout << "( ";
    for (int i = 0; i < sz + 1; i++)
      inorder(get(tmp[i]));
    cout << ") ";
  } else {
    free(n.id);
  }
}

template<class T>
void BTree<T>::preorder(BNode<T>& n) {
  T tkeys[CHUNK_SIZE+1];
  int tnext[CHUNK_SIZE+2];
  int sz=n.numkeys;
  if (n.leaf()) {
    for (int i = 0; i < sz; i++)
      cout << n.keys[i] << endl;
    free(n.id);
    return;
  }
  memcpy(tkeys, n.keys, sizeof(T) * (CHUNK_SIZE+1));
  memcpy(tnext, n.next, sizeof(int) * (CHUNK_SIZE+2));

  free(n.id);

  for (int i = 0; i < sz; i++) {
    preorder(get(tnext[i]));
    cout << tkeys[i] << endl;
  }
  preorder(get(tnext[sz]));
}
template<class T>
void BTree<T>::inorder() {
  inorder(manager.get_root());
  cout << endl;
}
template<class T>
void BTree<T>::preorder() {
  preorder(manager.get_root());
}
