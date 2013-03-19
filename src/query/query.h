#ifndef QUERY_QUERY_H_
#define QUERY_QUERY_H_
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <string>
#include <vector>

using namespace std;

enum Sign {
  SIGN_SINGLE,  // single token, such as 'a', 'b', 'c' below
  SIGN_AND,     // a & b & c
  SIGN_OR,      // a | b |c
  SIGN_NOT,     // ! a
  SIGN_PHRSE,   // "a b c"
  SIGN_NEAR     // a \n b
};

/**
 * A query gramma tree, leaf nodes are tokens, with sign=SIGN_SINGLE,
 * while non-leaf ones are operators
 *        OR
 *       /  \
 *      AND NOT
 *     / \  |
 *    Q1 Q2 Q3
 * (tree for Q1 & Q2 | ! Q3)
 */
class Query {
 public:
  Sign sign;      // operator
  string info;    // extra info for current operator, such as \n
  string token;   // when operator is null, token of current node (unstemmed)
  vector<int> hit_docs;

 private:
  vector<Query*> children;  // subtrees

 public:
  Query(Sign s);
  Query(string t);
  ~Query();

  void add(Query* n);
  unsigned size();
  void clear();
  Query* get(int i);
  vector<int>& docs();
  void optimize();  // only when hit_docs are not null
  string tostring();
  void dump();
};

#endif  // QUERY_QUERY_H
