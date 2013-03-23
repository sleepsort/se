#ifndef SEARCH_SUGGESTER_H_
#define SEARCH_SUGGESTER_H_
#include <utility>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include "index/reader.h"
#include "query/query.h"
class Suggester {
 public:
  Suggester(IndexReader &r);
  ~Suggester();
  bool suggest(Query *q);
  bool match(string &w);
  void kgram(string &w, vector<int> &collect);
  void levenrank(string &w, vector<int> &collect);

 private:
  IndexReader *ir;  // only use: widmap, wordmap & grams
};

#endif  // SEARCH_SUGGESTER_H_
