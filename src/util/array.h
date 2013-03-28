#ifndef UTIL_UTIL_H_
#define UTIL_UTIL_H_
#include <iostream>
#include <string>
#include <vector>
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


void encode_vb(const int* raw, int num, char *data, int &size);
void decode_vb(const char *data, int size, int *raw, int &num);


void dump(vector<int> &a);
void dump(vector<pair<int, int> > &a);

#endif  // UTIL_UTIL_H_
