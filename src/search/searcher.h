#ifndef SEARCHER_H
#define SEARCHER_H
#include "index/reader.h"
#include "query/query.h"
#include "query/parser.h"
#include "util/util.h"

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
#endif
