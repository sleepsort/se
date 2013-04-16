#ifndef SEARCH_SCORER_H_
#define SEARCH_SCORER_H_

#include <cmath>
#include "query/query.h"
#include "index/reader.h"

enum Model {
  Model_VSM,     // vector space model
  Model_OKAPI,   // okapi bm25
  Model_LMJM,    // language model, JM smoothing
  Model_LMDIRI   // language model, Dirichlet smoothing 
};

class Scorer {
 public:
  Scorer(IndexReader &r);
  ~Scorer();
  void init(Query* q);

  vector<pair<int, double> > &score(Model model);

  double scoreOKAPI(int tid, int did);
  double scoreVSM(int tid, int did);
  double scoreLMJM(int tid, int did);
  double scoreLMDIRI(int tid, int did);

 private:
  IndexReader* ir;
  map<int, vector<int> > docs;
  vector<pair<int, double> > scores;
};
#endif  // SEARCH_SCORER_H_
