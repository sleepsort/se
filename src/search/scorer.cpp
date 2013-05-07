#include "search/scorer.h"
Scorer::Scorer(IndexReader &r) {
  this->ir = &r;
}
Scorer::~Scorer() {
}
void Scorer::init(Query* q) {
  docs.clear();
  switch(q->sign) {
  case SIGN_SINGLE: {
    string term = q->token;
    lowercase(term);
    porterstem(term);
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
      lowercase(term);
      porterstem(term);
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

typedef std::pair<int, float> s_pair;
bool sort_pred(const s_pair& l, const s_pair& r) {
  if (l.second == r.second)
    return l.first < r.first;
  return l.second > r.second;
}

// NOTE: this is actually a generalized method, 
// might be reused for bm25, lm etc... 

float Scorer::scoreVSM(int tid, int did) {
  ir->filldoc(tid);
  ir->fillpos(tid, did);
  int N = ir->didmap.size();
  int tf = ir->postings[tid][did];
  int df = ir->tidmap[tid].df;
  int len = ir->didmap[did].len;
  if (len == 0) {
    return 0;
  } else { 
    return (double)tf * log(N/df) / len; 
  }
}

float Scorer::scoreOKAPI(int tid, int did) {
  ir->filldoc(tid);
  ir->fillpos(tid, did);
  int N = ir->didmap.size();
  int tf = ir->postings[tid][did];
  int df = ir->tidmap[tid].df;
  int len = ir->didmap[did].len;
  double avl = double(ir->ttf) / N;
  double idf = log( (N-df+0.5) / (df+0.5) );
  double k = 1.2;
  double b = 0.4;
  if (idf < 0) 
    return 0;
  return idf * (tf * (k+1)) / (tf + k * (1- b + b * len / avl));
}

float Scorer::scoreLMJM(int tid, int did) {
  ir->filldoc(tid);
  ir->fillpos(tid, did);
  long long ttf = ir->ttf; 
  int cf = ir->tidmap[tid].cf;
  int tf = ir->postings[tid][did];
  int len = ir->didmap[did].len;
  double col_prob = double(cf) / ttf;
  double doc_prob = double(tf) / len;
  double lambda = 0.5;
  return log( (1-lambda) * doc_prob + lambda * col_prob );
}

float Scorer::scoreLMDIRI(int tid, int did) {
  ir->filldoc(tid);
  ir->fillpos(tid, did);
  long long ttf = ir->ttf; 
  int cf = ir->tidmap[tid].cf;
  int tf = ir->postings[tid][did];
  int len = ir->didmap[did].len;
  double col_prob = double(cf) / ttf;
  double doc_prob = double(tf) / len;
  double mu = 1000;
  double lambda = mu / (mu + len);
  return log( (1-lambda) * doc_prob + lambda * col_prob );
}

// TODO: should the user be responsible to pass
// a vector inside? current framework is not thread-safe
vector<pair<int, float> >& Scorer::score(Model model) {
  // VSM model, DAAT
  map<int, vector<int> >::iterator it;
  set<pair<int, int> > heap;
  map<int, float> buf;
  map<int, int> upto;
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
    while (!heap.empty() && (*jt).first == head.first) {
      int did = (*jt).first;
      int tid = (*jt).second;

      switch (model) {
      case Model_VSM:
        buf[did] += scoreVSM(tid, did);
        break;
      case Model_OKAPI:
        buf[did] += scoreOKAPI(tid, did);
        break;
      case Model_LMJM:
        buf[did] += scoreLMJM(tid, did);
        break;
      case Model_LMDIRI:
        buf[did] += scoreLMDIRI(tid, did);
        break;
      }

      vector<int> &v = docs.find(tid)->second;
      if ((unsigned)upto[tid] < v.size()) {
        heap.insert(make_pair(v[upto[tid]], tid));
        upto[tid] += 1;
      }
      heap.erase(jt);
      jt = heap.begin();
    }
  }
  scores.clear();
  map<int, float>::iterator lt;
  for (lt = buf.begin(); lt != buf.end(); lt++) {
    scores.push_back(make_pair(lt->first, lt->second));
  }
  sort(scores.begin(), scores.end(), sort_pred);
  return scores;
}
