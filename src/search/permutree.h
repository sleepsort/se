#ifndef SEARCH_PERMUTERM_H_
#define SEARCH_PERMUTERM_H_
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include "template/btree.h"
#include "util/head.h"
using namespace std;

class Permuterm {
 public:
  int tid;
  char buf[PERMU_BUF + 1];
 public:
  Permuterm() {}
  Permuterm(const string &s, int id) {
    tid = id;
    memcpy(buf, s.c_str(), sizeof(buf));
  }
  Permuterm(const Permuterm &p) {
    memcpy(buf, p.buf, sizeof(buf));
  }
  inline bool operator==(const Permuterm& n){return !strcmp(buf,n.buf);}
  inline bool operator!=(const Permuterm& n){return !operator==(n);} 
  inline bool operator< (const Permuterm& n){return strcmp(buf,n.buf)<0;} 
  inline bool operator> (const Permuterm& n){return strcmp(buf,n.buf)>0;} 
  inline bool operator<=(const Permuterm& n){return !operator> (n);} 
  inline bool operator>=(const Permuterm& n){return !operator< (n);}
};

class PermutermTree : public BTree<Permuterm> {
 public:
  PermutermTree(const string& prefix) : BTree<Permuterm>(prefix) {}
  void search(string& token, vector<string> &collect);
  void rotate(string& token, char delimiter);
};
#endif  // SEARCH_PERMUTERM_H_
