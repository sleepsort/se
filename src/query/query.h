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
  SIGN_SINGLE,    // single token, such as 'a', 'b', 'c' below
  SIGN_WILDCARD,  // x*y
  SIGN_AND,       // a & b & c
  SIGN_OR,        // a | b |c
  SIGN_NOT,       // ! a
  SIGN_PHRSE,     // "a b c"
  SIGN_NEAR       // a \n b
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
  string attr;    // extra info for current operator, such as \n
  string token;   // when operator is null, token of current node (unstemmed)

 private:
  vector<Query*> children;  // subtrees
  vector<int> hit_docs;
  vector<float> hit_scores;

 public:
  Query(Sign s);
  Query(const string &t);
  ~Query();

  void clear();
  void optimize();  // only when hit_docs are not null
  void add(Query* n);
  void copy(Query* n);
  Query* get(int i);

  unsigned size();
  vector<int>& docs();
  vector<float>& scores();

  string tostring();
};

#endif  // QUERY_QUERY_H_
