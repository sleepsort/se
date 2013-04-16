#include "search/scorer.h"
Scorer::Scorer(IndexReader &r) {
  this->ir = &r;
}
Scorer::~Scorer() {
}
void Scorer::init(Query* q) {
  switch(q->sign) {
  case SIGN_SINGLE: {
    string term = q->token;
    if (ir->termmap.find(term) == ir->termmap.end())
      return;
    int qid = ir->termmap[term];
    docs[qid].insert(docs[qid].begin(), q->docs().begin(), q->docs().end());
    break;
  }
  case SIGN_OR: {
    q->optimize();
    for (unsigned i = 0; i < q->size(); ++i) {
      string term = q->get(i)->token;
      if (ir->termmap.find(term) == ir->termmap.end())
        continue;
      int qid = ir->termmap[term];
      vector<int> &v = q->get(i)->docs();
      docs[qid].insert(docs[qid].begin(), v.begin(), v.end());
    }
    break;
  }
  default: {
    error("Scorer::Not Implemented Yet");
    break;
  }
  }
}

typedef std::pair<int, double> s_pair;
bool sort_pred(const s_pair& l, const s_pair& r) {
  if (l.second == r.second)
    return l.first < r.first;
  return l.second > r.second;
}

vector<pair<int, double> >& Scorer::score() {
  map<int, vector<int> >::iterator it;
  set<pair<int, int> > heap;
  map<int, int> upto;
  map<int, double> buf;
  for (it= docs.begin(); it != docs.end(); it++) {
    int tid = it->first;
    int did = it->second[0];
    heap.insert(make_pair(did, tid));
    buf[did] = 0;
    upto[tid] = 1;
  }
  while (!heap.empty()) {
    set<pair<int, int> >::iterator jt = heap.begin();
    pair<int, int> head = *jt;
    while ((*jt).first == head.first) {
      int did = (*jt).first;
      int tid = (*jt).second;
      ir->filldoc(tid);
      ir->fillpos(tid, did);
      int tf = ir->postings[tid][did].size();
      int df = ir->tidmap[tid].df;
      assert (df > 0);

      buf[did] += (double)tf / log(df); 

      vector<int> &v = docs.find(tid)->second;
      if ((unsigned)upto[tid] < v.size()) {
        heap.insert(make_pair(v[upto[tid]], tid));
        upto[tid] += 1;
      }
      heap.erase(jt);
      if (heap.empty()) {
        break;
      } else {
        jt = heap.begin();
      }
    }
  }
  map<int, double>::iterator lt;
  for (lt = buf.begin(); lt != buf.end(); lt++) {
    int len = ir->didmap[lt->first].len;
    assert(len > 0);
    scores.push_back(make_pair(lt->first, lt->second / len));
  }
  sort(scores.begin(), scores.end(), sort_pred);
  return scores;
}
