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

// assumption on max word/term length
#define WORD_BUF 1024
// assumption on max file line length
#define LINE_BUF  1024000

void tokenize(string file, vector<string> &collect);
void collect(string path, vector<string> &files, set<string> &exclude);

void lowercase(string &s);
void porterstem(string &s);

int levendistance(const string &s1, const string &s2);

void conjunct(vector<int>& a, vector<int>& b, vector<int>& c);
void disjunct(vector<int>& a, vector<int>& b, vector<int>& c);
void diff(vector<int>& a, vector<int>& b, vector<int>& c);

void dump(vector<int>& a);

#endif
