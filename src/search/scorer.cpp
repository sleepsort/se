#include "search/scorer.h"
Scorer::Scorer(Query &q, IndexReader &r) {
  this->q = &q;
  this->ir = &r;
}
Scorer::~Scorer() {
}
void Scorer::init() {
  switch(q->sign) {
  case SIGN_OR:
    q->optimize();
    for (unsigned i = 0; i < q->size(); ++i) {
      string term = q->get(i)->token;
      if (ir->termmap.find(term) == ir->termmap.end())
        continue;
      int qid = ir->termmap[term];
      vector<int> &dlist = q->get(i)->docs();
      docs[qid].insert(docs[qid].begin(), dlist.begin(), dlist.end());
    }
    break;
  default:
    error("Scorer::Not Implemented Yet");
    break;
  }
}
