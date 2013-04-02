#ifndef SEARCH_PERMUTERM_H_
#define SEARCH_PERMUTERM_H_
#include <cstring>
#include <iostream>
#include <string>
#include <set>
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

class PermuTree : public BTree<Permuterm> {
 public:
  PermuTree() : BTree<Permuterm>() {}
  void init(const string &prefix);
  void search(string& token, set<int> &collect);
  static void rotate(string& token, char delimiter);
  static void increase(string& token);
};
#endif  // SEARCH_PERMUTERM_H_
