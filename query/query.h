#ifndef QUERY_H
#define QUERY_H

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace std;

enum Sign {
    SIGN_NULL,
    SIGN_AND,
    SIGN_OR,
    SIGN_NOT
};

/**
 * A query gramma tree, leaf nodes are tokens, with sign=SIGN_NULL, 
 * while non-leaf ones are operators
 *        OR
 *       /  \
 *      AND NOT
 *     / \  |
 *    Q1 Q2 Q3
 * (tree for Q1 AND Q2 OR NOT Q3)
 */
class Query { 
public:
    Sign sign;     // operator
    string token;  // when operator is null, token of current node
    vector<int> hit_docs;

private:
    vector<Query*> children; // subtrees

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
    void dump();
};

#endif
