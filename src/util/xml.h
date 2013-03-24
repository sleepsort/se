#ifndef UTIL_XML_H_
#define UTIL_XML_H_
#include <iostream>
#include <string>
#include <vector>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "util/string.h"
using namespace std;

void xmltokenize(const string &file, vector<string> &collect);

#endif  // UTIL_XML_H_
