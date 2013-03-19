#ifndef SEARCH_SEARCHER_H_
#define SEARCH_SEARCHER_H_
#include <string>
#include <vector>
#include <map>
#include "query/parser.h"
#include "query/query.h"
#include "util/util.h"
#include "index/reader.h"

class IndexSearcher {
 public:
  IndexSearcher(IndexReader &r);
  ~IndexSearcher();
  void search(Query *q);
  void report(Query *q);

 private:
  void searchSINGLE(Query *q);
  void searchAND(Query *q);
  void searchOR(Query *q);
  void searchNOT(Query *q);
  void searchPHRSE(Query *q);
  void searchNEAR(Query *q);

 private:
  IndexReader* ir;
  vector<int> alldoc;
};
#endif  // SEARCH_SEARCHER_H_
