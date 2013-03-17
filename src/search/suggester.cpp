#include "search/suggester.h"
Suggester::Suggester(IndexReader &r) {
  this->ir = &r;
}
Suggester::~Suggester() {
}
bool Suggester::match(string &w) {
  return (ir->wordmap.find(w) != ir->wordmap.end());
}
void Suggester::kgram(string &w, vector<int>& collect) {
  vector<vector<int>*> candidates;
  unsigned sz = w.length();

  for (int k = IndexWriter::MIN_N_GRAM; k <= IndexWriter::MAX_N_GRAM; ++k) {
    for (unsigned n = 0; n < sz - 1; ++n) {
      string gram = w.substr(n, k);
      map<string, vector<int> >::iterator it = ir->grams.find(gram);
      if (it != ir->grams.end()) {
        candidates.push_back(&(it->second));
      }
    }
  }
  if (candidates.empty())
    return;
  vector<pair<int, int> > foo;
  for (unsigned i = 0; i < candidates.size(); ++i) {
     vector<pair<int, int> > bar;
     for (unsigned j = 0; j < candidates[i]->size(); ++j) {
      bar.push_back(make_pair(1, (*candidates[i])[j]));
     }
     disjunct(foo, bar, foo);
  }
  for (unsigned i = 0; i < foo.size(); ++i) {
    pair<int, int> &p = foo[i];
    // (sz - 1) means exact match
    if (p.first >= static_cast<int>(sz) - 3) {
      collect.push_back(p.second);
    }
  }
}
void Suggester::levenrank(string &w, vector<int> &collect) {
  vector<pair<int, int> > foo;
  int dist;
  for (unsigned i = 0; i < collect.size(); ++i) {
    dist = levendistance(w, ir->vidmap[collect[i]]);
    foo.push_back(make_pair(dist, collect[i]));
  }
  sort(foo.begin(), foo.end());
  for (unsigned i = 0; i < foo.size(); ++i) {
    collect[i] = foo[i].second;
  }
}
