#ifndef SEARCH_SCORER_H_
#define SEARCH_SCORER_H_
#include "query/query.h"
#include "index/reader.h"

class Scorer {
 public:
  Scorer(Query &q, IndexReader &r);
  ~Scorer();
  void init();

 private:
  IndexReader* ir;
  Query* q;
  map<int, vector<int> > docs;
  map<int, double> scores;
};
#endif  // SEARCH_SCORER_H_
