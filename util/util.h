#ifndef UTIL_H
#define UTIL_H
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>
#include <set>
using namespace std;

void tokenize(string file, vector<string> &collect);
void collect(string path, vector<string> &files, set<string> &exclude);

void conjunct(vector<int>& a, vector<int>& b, vector<int>& c);
void disjunct(vector<int>& a, vector<int>& b, vector<int>& c);
void diff(vector<int>& a, vector<int>& b, vector<int>& c);

void dump(vector<int>& a);

#endif
