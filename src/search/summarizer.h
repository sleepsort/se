#ifndef SEARCH_SUMMARIZER_H_
#define SEARCH_SUMMARIZER_H_

#include <string>
#include "index/reader.h"
#include "index/loader.h"
#include "query/query.h"

class Summarizer {
 public:
  Summarizer(IndexReader &r, FileLoader &fl);
  ~Summarizer();
  string summary(Query *q, int did);

 private:
  void cleanbody(string &body);
  string highlight(const string &body, const vector<string> &query);
  void mark(const string &body, const vector<string> &query,
            vector<pair<int, int> > &offsets);
  pair<int, int> maxspan(const vector<pair<int, int> > &offsets);
  pair<int, int> fixrange(const string &body, const pair<int, int> ori);

 private:
  static const char COLOR_RESET[];
  static const char COLOR_RED[];
  static const char COLOR_GREEN[];
  static const char COLOR_BROWN[];
  static const char COLOR_BLUE[];
  static const char COLOR_GREY[];
  static const char COLOR_BOLD[];
  static const int MAX_RANGE;

  IndexReader *ir;  // only use didmap
  FileLoader *fl;   // only use seek() and body()
};
#endif  // SEARCH_SUMMARIZER_H_
