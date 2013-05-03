#ifndef UTIL_ARRAY_H_
#define UTIL_ARRAY_H_
#include <sys/time.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <set>
#include "util/head.h"
using namespace std;

void conjunct(vector<int> &a, vector<int> &b, vector<int> &c);
void disjunct(vector<int> &a, vector<int> &b, vector<int> &c);
void diff(vector<int> &a, vector<int> &b, vector<int> &c);

void disjunct(vector<pair<int, int> > &a, vector<pair<int, int> > &b,
              vector<pair<int, int> > &c);

int array_insert(int* a, int len, int key, int pos);
int bsearch(const int *a, int len, int key);

template <class R, class T>
void dump(vector<pair<R, T> > &a) {
  for (unsigned i = 0; i < a.size(); ++i) {
    cout << " <" << a[i].first  <<  ","  << a[i].second << "> ";
  }
  cout << endl;
}

template <class T>
void dump(vector<T> &a) {
  for (unsigned i = 0; i < a.size(); ++i) {
    cout << a[i] << " ";
  }
  cout << endl;
}

void array_expand(void **p, int& oldlen, int newlen);

#endif  // UTIL_ARRAY_H_
