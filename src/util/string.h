#ifndef UTIL_STRING_H_
#define UTIL_STRING_H_
#include <utility>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "util/head.h"
#include "util/porter.h"
using namespace std;

void tokenize(char *c, vector<string> &collect);

void lowercase(string &s);
void porterstem(string &s);

string itoa(int number);

int levendistance(const string &s1, const string &s2);

#endif  // UTIL_STRING_H_
