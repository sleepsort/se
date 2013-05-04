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

bool nodigit(const char *str);

void lowercase(string &s);
void porterstem(string &s);


char *trim(char *str);
string itoa(int number);

const char *byte2bin(char x);

int levendistance(const string &s1, const string &s2);

#include <sstream>
template <typename T>
string tostring (T n) {
  ostringstream ss;
  ss << n;
  return ss.str();
}

#endif  // UTIL_STRING_H_
