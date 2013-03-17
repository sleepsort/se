#ifndef SUGGESTER_H
#define SUGGESTER_H
#include "index/reader.h"
class Suggester {
public:
  Suggester(IndexReader &r);
  ~Suggester();
  bool match(string &w);
  void kgram(string& w, vector<int>& collect);
  void levenrank(string& w, vector<int>& collect);

private:
  IndexReader *ir;
};

#endif 
