#include "template/btree.h"

/*-------- BNode--------*/

template<class T>
BNode<T>::BNode() {
  this->id = -1;
  this->leaf = 1;
  this->numkeys = 0;
  this->sibling = -1;
}
template<class T>
BNode<T>::~BNode() {
}
template<class T>
void BNode<T>::init(int nid) {
  this->id = nid;
  this->leaf = 1;
  this->numkeys = 0;
  this->sibling = -1;
  this->next[CHUNK_SIZE+1] = 0x53535353;
}
template<class T>
int BNode<T>::findkey(T& key) {
  int f = 0, t = numkeys -1, m = 0, cmp = 0;
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
  assert(!leaf);
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
  assert(leaf);
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


/*-------- BManager--------*/

template<class T>
BManager<T>::BManager() {
  this->num_nodes = 0;
  this->root_node_id = -1;   // should be fetched from file
  this->num_data = 0;
  this->optimized = 0;
  memset(bitmap, PAGE_NULL, sizeof(bitmap[0]) * MEMORY_BUFF);
  memset(pool, -1, sizeof(pool[0]) * MEMORY_BUFF);
}
template<class T>
BManager<T>::~BManager() {
  if (!optimized) {
    optimize_data();
  }
  fclose(data_file);

  // flush all changed, or new pages to disk
  map<int, int>::iterator it;
  for (it = nodemap.begin(); it != nodemap.end(); ++it) {
    int nodeid = it->first;
    int pageid = it->second;
    if ((bitmap[pageid] & PAGE_DIRTY) && pool[pageid].id == nodeid) {
      flush(nodeid);
    }
  }
  fclose(node_file);
  
  meta_file = fopen(meta_path.c_str(), "w");
  fprintf(meta_file, "%d\n", num_nodes);
  fprintf(meta_file, "%d\n", root_node_id);
  fprintf(meta_file, "%d\n", num_data);
  fprintf(meta_file, "%d\n", optimized);
  for (int i = 0 ; i < num_data; i++) {
    fprintf(meta_file,"%lld %d\n",data_field[i].first, data_field[i].second);
  }
  fclose(meta_file);
}


template<class T>
void BManager<T>::init(string &meta_path, string &node_path, string &data_path) {
  this->meta_path = meta_path;
  this->node_path = node_path;
  this->data_path = data_path;

  meta_file = fopen(meta_path.c_str(), "r");
  if (meta_file) {
    long long fp;
    int len, r;
    if ((r = fscanf(meta_file, "%d", &num_nodes)) < 0) {
      fprintf(stderr,"error loading numnodes\n");
    }
    if ((r = fscanf(meta_file, "%d", &root_node_id)) < 0) {
      fprintf(stderr,"error loading root_id\n");
    }
    if ((r = fscanf(meta_file, "%d", &optimized)) < 0) {
      fprintf(stderr,"error loading optimize info\n");
    }
    if ((r = fscanf(meta_file, "%d", &num_data)) < 0) {
      fprintf(stderr,"error loading numdata\n");
    }
    for (int i = 0 ; i < num_data; i++) {
      if ((r = fscanf(meta_file,"%lld %d\n", &fp, &len)) >= 0) {
        data_field.push_back(make_pair(fp,len));
      }
    }
    fclose(meta_file);
  } else {
    node_file = fopen(node_path.c_str(), "w");
    data_file = fopen(data_path.c_str(), "w");
    if (node_file) {
      fclose(node_file);
    }
    if (data_file) {
      fclose(data_file);
    }
  }
  node_file = fopen(node_path.c_str(), "rb+");
  data_file = fopen(data_path.c_str(), "rb+");
  assert(node_file != NULL && data_file != NULL);
}

template<class T>
void BManager<T>::dump() {
  cout << endl;
  for (int i=0; i<MEMORY_BUFF; i++)
    cout << bitmap[i] << " ";
  cout << endl;
  for (int i=0; i<MEMORY_BUFF; i++)
    cout << pool[i].id << " ";
  cout << endl;
}

template<class T>
BNode<T>& BManager<T>::new_node() {
  int pageid = allocate();
  int nodeid = num_nodes++;

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
  if (num_nodes == 0)
    return new_root();
  return get_node(root_node_id);
}
template<class T>
BNode<T>& BManager<T>::get_node(int id) {
  int pageid;
  if (nodemap.find(id) == nodemap.end() || pool[nodemap[id]].id != id) {
    pageid = allocate();
    assert(pageid >= 0);
    nodemap[id] = pageid;
    load(id);
  } else {
    pageid = nodemap[id];
    bitmap[pageid] |= PAGE_LOCK;
  }
  return pool[pageid];
}
// Mark current node as 'soft free', maybe
// reused if the id is not changed.
// Never free root node even the caller 
// mistakenly returns it.
template<class T>
void BManager<T>::return_node(int id) {
  int pageid = nodemap[id];
  if (id != root_node_id) {
    bitmap[pageid] &= (~PAGE_LOCK);
  }
}

// Should mark updated node as dirty,
// when manager is closed, or when the 
// node is scheduled out of memory, flush
// it to disk
template<class T>
void BManager<T>::update_node(int id) {
  bitmap[nodemap[id]] |= PAGE_DIRTY;
}

template<class T>
int BManager<T>::new_data(void* data, int length) {
  if (length <= 0)
    return -1;
  int id = num_data;
  data_field.push_back(make_pair(ftell(data_file), length));
  fseek(data_file, datafp(id), SEEK_SET);
  fwrite(data, length, 1, data_file);
  num_data++;
  optimized = 0;
  return id;
}

// TODO(billy): should the user be responsible for the
// new operation ?
template<class T>
void* BManager<T>::get_data(int id, int &length) {
  if (id >= num_data) {
    length = 0;
    return NULL;
  }
  length = data_field[id].second;
  char *tmp = new char[length];
  int r;
  fseek(data_file, datafp(id), SEEK_SET);
  if ((r = fread(tmp, length, 1, data_file)) <= 0) {
    delete tmp;
    return NULL;
  }
  return tmp;
}
template<class T>
void BManager<T>::optimize_data() {
  cout << "optimizing" << endl;
  return;
  vector<pair<long long, int> > backup;
  FILE *tmp_file = fopen((data_path+".tmp").c_str(), "w");
  int buf_len = 1024, r;
  char *buf = new char[buf_len];
  backup.swap(data_field);
  for (int i = 0; i < num_data; i++) {
    long long old_fp = backup[i].first, new_fp = ftell(tmp_file);
    int len = backup[i].second;
    fseek(data_file, old_fp, SEEK_SET);
    if (buf_len < len) {
      delete buf;
      buf_len = len;
      buf = new char[buf_len];
    }
    if ((r = fread(buf, len, 1, data_file)) <= 0) {
      fprintf(stderr, "error arranging data\n");
      return;
    }
    fwrite(buf, 1, len, tmp_file);
    data_field.push_back(make_pair(new_fp, len));
  }
  optimized = 1;
  fclose(tmp_file);
  fclose(data_file);
  remove(data_path.c_str());
  rename((data_path+".tmp").c_str(), data_path.c_str());
  data_file = fopen(data_path.c_str(), "rb+");
  delete buf;
}


template<class T>
int BManager<T>::allocate() {
  int last = -1;
  for (int i = 0; i < MEMORY_BUFF; ++i) {
    if (bitmap[i] & PAGE_DIRTY) {
      if (!(bitmap[i] & PAGE_LOCK)) {
        last = i;
      }
    } else if (!(bitmap[i] & PAGE_LOCK)) {
      bitmap[i] |= PAGE_LOCK;
      return i;
    }
  }
  if (last >= 0) {
    flush(pool[last].id);
    bitmap[last] &= (~PAGE_DIRTY);
    bitmap[last] |= PAGE_LOCK;
    return last;
  }
  return -1;
}

template<class T>
long long BManager<T>::nodefp(int id) {
  return NODE_SZ * id;
}

template<class T>
long long BManager<T>::datafp(int id) {
  if (num_data == 0 || id > num_data) 
    return 0;
  pair<long long, int> &info = data_field[id-1];
  return info.first+info.second;
}

// file will gracefully extend when reaching a new max_id
template<class T>
void BManager<T>::flush(int id) {
  fseek(node_file, nodefp(id), SEEK_SET);
  fwrite((void*)&pool[nodemap[id]], NODE_SZ, 1, node_file);
}
template<class T>
void BManager<T>::load(int id) {
  int r;
  fseek(node_file, nodefp(id), SEEK_SET);
  if ((r = fread((void*)&pool[nodemap[id]], NODE_SZ, 1, node_file)) <= 0) {
    fprintf(stderr, "error loading node: %d\n", id);
  }
}


/*-------- BTree --------*/

template<class T>
BTree<T>::BTree(string &metapath, string &nodepath, string &datapath) {
  this->manager.init(metapath, nodepath, datapath);
}
template<class T>
BTree<T>::~BTree() {
}

// Split node as two.
// For non-leaf node, this procedure will make one key ascended
// For leaf node, the key will only be copied up
template<class T>
void BTree<T>::split(int p_id, int n_id) {
  BNode<T>& n = get_node(n_id);
  BNode<T>& t = manager.new_node();
  int pos = n.ascendpos();
  T& newkey = n.keys[pos];
  t.leaf = n.leaf;
  t.sibling = n.sibling;
  t.numkeys = n.numkeys - pos - 1;
  memcpy(t.keys, &(n.keys[pos+1]), sizeof(T)*(t.numkeys));
  if (!n.leaf) {
    n.numkeys = pos;  // key ascended
    memcpy(t.next, &(n.next[pos+1]), sizeof(int)*(t.numkeys+1));
  } else {
    n.numkeys = pos + 1;  // key copied
    n.sibling = t.id; 
    memcpy(t.next, &(n.next[pos+1]), sizeof(int)*(t.numkeys));
  }
  if (p_id == -1) {  // root splits
    BNode<T>& pp = manager.new_root();
    pp.leaf = 0;
    pp.sibling = -1;
    p_id = pp.id;
    return_node(pp.id);
  }
  BNode<T>& p = get_node(p_id);
  int newpos = p.findkey(newkey);
  p.addkey(newkey, newpos);
  p.addnext(n.id, t.id, newpos);
  p.numkeys++;
  update_node(n.id); return_node(n.id);
  update_node(t.id); return_node(t.id);
  update_node(p.id); return_node(p.id);
  return;
}


// Insert key to the tree, 
// duplicate key will be omited
template<class T>
void BTree<T>::insert(T& key, void *data=NULL, int length=0) {
  int rootid = manager.get_root().id;
  return_node(rootid);
  insert(-1, rootid, key, data, length);
}

template<class T>
void BTree<T>::insert(int p_id, int n_id, T& key, void *data, int length) {
  BNode<T> &n = get_node(n_id);
  int pos = n.findkey(key), sz = n.numkeys;
  if (pos < sz && key == n.keys[pos]) { // duplicated
    return;
  }
  if (n.leaf && n.addkey(key, pos) >= 0) {
    n.adddata(manager.new_data(data,length), pos);
    n.numkeys++;
    update_node(n_id);
    return_node(n_id);
  } else {
    int m_id = n.next[pos];
    return_node(n_id);
    insert(n_id, m_id, key, data, length);
  }
  BNode<T> &nn = get_node(n_id);
  if (nn.numkeys >= CHUNK_SIZE) {
    return_node(n_id);
    split(p_id, n_id);
  } else {
    return_node(n_id);
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
  int ret;
  while (true) {
    BNode<T>& cur = manager.get_node(cur_id);
    int i = cur.findkey(key);
    if (cur.leaf) {
      if (i >= cur.numkeys) {
        ret = force ? cur_id : -1;
      } else {
        ret = cur_id;
      }
      return_node(cur.id);
      return ret;
    }
    cur_id = cur.next[i];
    return_node(cur.id);
  }
}

template<class T>
int BTree<T>::search_node(T& key) {
  return search(key, false);
}

template<class T>
int BTree<T>::search_data(T& key) {
  int nodeid = search_node(key);
  if (nodeid < 0)
    return -1;
  BNode<T> &node = get_node(nodeid);
  int pos = node.findkey(key);
  assert(pos >= 0);
  return node.next[pos];
}



template<class T>
BNode<T>& BTree<T>::get_node(int id) {
  return manager.get_node(id);
}
template<class T>
void BTree<T>::return_node(int id) {
  manager.return_node(id);
}
template<class T>
void BTree<T>::update_node(int id) {
  manager.update_node(id);
}
template<class T>
void* BTree<T>::get_data(int id, int &len) {
  return manager.get_data(id, len);
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
  if (!n.leaf) {
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
  int sz = n.numkeys;
  if (!n.leaf) {
    memcpy(tmp, n.next, sizeof(int) * (CHUNK_SIZE+2));
    return_node(n.id);
    cout << "( ";
    for (int i = 0; i < sz + 1; i++)
      inorder(get_node(tmp[i]));
    cout << ") ";
  } else {
    return_node(n.id);
  }
}

template<class T>
void BTree<T>::preorder(BNode<T>& n) {
  T tkeys[CHUNK_SIZE+1];
  int tnext[CHUNK_SIZE+2];
  int sz=n.numkeys;
  if (n.leaf) {
    for (int i = 0; i < sz; i++)
      cout << n.keys[i] << endl;
    return_node(n.id);
    return;
  }
  memcpy(tkeys, n.keys, sizeof(T) * (CHUNK_SIZE+1));
  memcpy(tnext, n.next, sizeof(int) * (CHUNK_SIZE+2));

  return_node(n.id);

  for (int i = 0; i < sz; i++) {
    preorder(get_node(tnext[i]));
  //  cout << tkeys[i] << endl;
  }
  preorder(get_node(tnext[sz]));
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
