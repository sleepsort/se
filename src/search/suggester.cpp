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
    if (p.first >= static_cast<int>(sz) - 4) {
      collect.push_back(p.second);
    }
  }
}
void Suggester::levenrank(string &w, vector<int> &collect) {
  vector<pair<int, int> > foo;
  int dist;
  for (unsigned i = 0; i < collect.size(); ++i) {
    dist = levendistance(w, ir->widmap[collect[i]]);
    foo.push_back(make_pair(dist, collect[i]));
  }
  sort(foo.begin(), foo.end());
  for (unsigned i = 0; i < foo.size(); ++i) {
    collect[i] = foo[i].second;
  }
}

// will only try to correct token for each leaf node 
// (SINGLE Query) and backup original token on Query::attr field, then 
// run it on the searcher
bool Suggester::suggest(Query *q) {
  if (q->sign == SIGN_SINGLE) {
    string t = q->token;
    if (match(t))  // correct word
      return false;
    vector<int> collect;
    kgram(t, collect);
    levenrank(t, collect);
    if (collect.size() > 0) {
      q->attr = q->token;
      q->token = ir->widmap[collect[0]];
      return true;
    }
    return false;
  }
  bool modified = false;
  for (unsigned i = 0; i < q->size(); ++i)
    if (suggest(q->get(i)))
      modified = true;
  return modified;
}
