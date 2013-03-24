#ifndef UTIL_FILE_H_
#define UTIL_FILE_H_
#include <dirent.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <utility>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "util/head.h"
using namespace std;

void tokenize(const string &file, vector<string> &collect);
void collect(const string &path, vector<string> &files, set<string> &exclude);

void fread(ifstream &fin, void *buf, size_t len);
void fpeek(ifstream &fin, void *buf, size_t len);
void fseekg(ifstream &fin, long long offset, ios_base::seekdir way);
long long ftellg(ifstream &fin);

void fwrite(ofstream &fout, void *buf, size_t len);
void fseekp(ofstream &fin, long long offset, ios_base::seekdir way);
long long ftellp(ofstream &fin);

#endif  // UTIL_FILE_H_
