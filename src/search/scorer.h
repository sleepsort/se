#ifndef SEARCH_SCORER_H_
#define SEARCH_SCORER_H_

#include <cmath>
#include "query/query.h"
#include "index/reader.h"

class Scorer {
 public:
  Scorer(IndexReader &r);
  ~Scorer();
  void init(Query* q);
  vector<pair<int, double> > &score();

 private:
  IndexReader* ir;
  map<int, vector<int> > docs;
  vector<pair<int, double> > scores;
};
#endif  // SEARCH_SCORER_H_
