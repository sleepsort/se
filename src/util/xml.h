#ifndef UTIL_XML_H_
#define UTIL_XML_H_

#include "../lib/tinyxml2.h"
#include <iostream>
#include <vector>
#include "util/string.h"
#include "head.h"

using std::string;
using std::vector;

void xmltokenize(const string &file, vector<string> &collect);

#endif  // UTIL_XML_H_
