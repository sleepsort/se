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

  vector<pair<int, float> > &score(Model model);

  float scoreOKAPI(int tid, int did);
  float scoreVSM(int tid, int did);
  float scoreLMJM(int tid, int did);
  float scoreLMDIRI(int tid, int did);

 private:
  IndexReader* ir;
  map<int, vector<int> > docs;
  vector<pair<int, float> > scores;
};
#endif  // SEARCH_SCORER_H_
