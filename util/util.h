#ifndef UTIL_H
#define UTIL_H
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <set>
#include "porter.h"
using namespace std;

// for stem only
#define WORD_BUF 1024
// for readling each line in one file
#define LINE_BUF  1024000

void tokenize(string file, vector<string> &collect);
void porterstem(string &s);
void collect(string path, vector<string> &files, set<string> &exclude);

void conjunct(vector<int>& a, vector<int>& b, vector<int>& c);
void disjunct(vector<int>& a, vector<int>& b, vector<int>& c);
void diff(vector<int>& a, vector<int>& b, vector<int>& c);

void dump(vector<int>& a);

#endif
