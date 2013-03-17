#ifndef UTIL_UTIL_H_
#define UTIL_UTIL_H_
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <utility>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "util/porter.h"
using namespace std;

// assumption on max word/term length
#define WORD_BUF 1024
// assumption on max file line length
#define LINE_BUF  1024000

void tokenize(const string &file, vector<string> &collect);
void collect(const string &path, vector<string> &files, set<string> &exclude);

void lowercase(string &s);
void porterstem(string &s);

int levendistance(const string &s1, const string &s2);

void conjunct(vector<int> &a, vector<int> &b, vector<int> &c);
void disjunct(vector<int> &a, vector<int> &b, vector<int> &c);
void diff(vector<int> &a, vector<int> &b, vector<int> &c);

void disjunct(vector<pair<int, int> > &a, vector<pair<int, int> > &b,
              vector<pair<int, int> > &c);


void dump(vector<int> &a);
void dump(vector<pair<int, int> > &a);

#endif  // UTIL_UTIL_H_
