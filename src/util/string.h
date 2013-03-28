#ifndef UTIL_STRING_H_
#define UTIL_STRING_H_
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "util/head.h"
#include "../lib/porter.h"
using namespace std;

void tokenize(const char *str, vector<string> &collect);

void lowercase(string &s);
void porterstem(string &s);

string itoa(int number);
const char *byte2bin(char x);

int levendistance(const string &s1, const string &s2);

#endif  // UTIL_STRING_H_
