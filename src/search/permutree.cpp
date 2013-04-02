#include "search/permutree.h"
void PermuTree::init(const string& prefix) {
  BTree<Permuterm>::init(prefix);
}
void PermuTree::search(string &token, set<int> &collect) {
  string fkey = token+"$", tkey;
  rotate(fkey,'*');
  tkey = fkey;
  increase(tkey);

  Permuterm fterm(fkey, 0), tterm(tkey, 0);
  assert(fkey < tkey);

  pair<int, int> node, pos;

  search_key_between(fterm, tterm, node, pos);
  int lid = node.first, rid = node.second;
  int lpos = pos.first, rpos = pos.second;
  assert(rid >= 0);

  while (lid != rid) {
    BNode<Permuterm> &n = get_node(lid);
    for (int i = lpos; i < n.numkeys; i++) {
      collect.insert(n.keys[i].tid);
    }
    lid = n.sibling;
    return_node(n.id);
    lpos = 0;
  }
  if (rid >= 0) {
    BNode<Permuterm> &n = get_node(rid);
    for (int i = lpos; i < rpos; i++) {
      collect.insert(n.keys[i].tid);
    }
    return_node(n.id);
  }
}
void PermuTree::rotate(string &t, char d) {
  string s = t;
  int sz = t.length();
  if (t[sz - 1] == d) {
    t = s.substr(0, sz-1);
    return;
  }
  for (int i = 0; i < sz - 1; i++) {
    if (s[i] == d) {
      t = s.substr(i+1);
      t += s.substr(0, i);
    }
  }
}
void PermuTree::increase(string &t) {
  int sz = t.length();
  t[sz-1] += 1;
}
