#include "search/searcher.h"
IndexSearcher::IndexSearcher(IndexReader &r) {
  this->ir = &r;
  map<int, string>::iterator it;
  for (it = ir->didmap.begin(); it != ir->didmap.end(); ++it) {
    this->alldoc.push_back(it->first);
  }
}
IndexSearcher::~IndexSearcher() {
}
void IndexSearcher::search(Query *q) {
  switch (q->sign) {
  case SIGN_SINGLE:
    searchSINGLE(q); return;
  case SIGN_WILDCARD:
    searchWILDCARD(q); return;
  default:
    break;
  }
  for (unsigned i = 0; i < q->size(); ++i)
    search(q->get(i));

  switch (q->sign) {
  case SIGN_NOT:
    searchNOT(q); break;
  case SIGN_AND:
    searchAND(q); break;
  case SIGN_OR:
    searchOR(q); break;
  case SIGN_PHRSE:
    searchPHRSE(q); break;
  case SIGN_NEAR:
    searchNEAR(q); break;
  default:
    break;
  }
}

void IndexSearcher::searchWILDCARD(Query *q) {
  // Here we'll modify the wildcard query 
  // into OR query
  set<int> hit;
  set<int>::iterator it;
  ir->permutree.search(q->token, hit);
  q->clear();
  for (it = hit.begin(); it != hit.end(); it++) {
    assert(ir->widmap.find(*it) != ir->widmap.end());
    string t = ir->widmap[*it];
    ir->permutree.rotate(t, '$');
    q->add(new Query(t));
  }
  if (q->size() > 0) {
    q->sign = SIGN_OR;
    search(q);
  } else {
    q->sign = SIGN_SINGLE;
  }
  // if wildcard matches no term, resort to suggester
}

void IndexSearcher::searchSINGLE(Query *q) {
  string t = q->token;
  lowercase(t);
  porterstem(t);

  if (ir->termmap.find(t) != ir->termmap.end()) {
    int tid = ir->termmap[t];
    map<int, map<int, vector<int> > >::iterator it;
    map<int, vector<int> >::iterator jt;
    ir->filldoc(tid);
    it = ir->postings.find(tid);
    if (it != ir->postings.end()) {
      vector<int> v;
      for (jt = it->second.begin(); jt != it->second.end(); ++jt) {
        v.push_back(jt->first);
      }
      q->docs().insert(q->docs().begin(), v.begin(), v.end());
    }
  }
}

void IndexSearcher::searchAND(Query *q) {
  q->optimize();
  disjunct(q->docs(), q->get(0)->docs(), q->docs());
  for (unsigned i = 1; i < q->size(); ++i)
    conjunct(q->docs(), q->get(i)->docs(), q->docs());
}
void IndexSearcher::searchOR(Query *q) {
  q->optimize();
  for (unsigned i = 0; i < q->size(); ++i)
    disjunct(q->docs(), q->get(i)->docs(), q->docs());
}
void IndexSearcher::searchNOT(Query *q) {
  diff(alldoc, q->get(0)->docs(), q->docs());
}

void IndexSearcher::searchPHRSE(Query *q) {
  vector<int> hits;

  disjunct(hits, q->get(0)->docs(), hits);
  for (unsigned i = 1; i < q->size(); ++i)
    conjunct(hits, q->get(i)->docs(), hits);

  if (q->size() == 1) {
    q->docs().insert(q->docs().begin(), hits.begin(), hits.end());
    return;
  }

  vector<int> phrase;
  bool success = true;

  for (unsigned i = 0; i < q->size(); ++i) {
    string t = q->get(i)->token;
    lowercase(t);
    porterstem(t);
    if (ir->termmap.find(t) == ir->termmap.end()) {
      return;
    }
    int tid = ir->termmap[t];
    phrase.push_back(tid);
  }
  for (unsigned k = 0; k < hits.size(); ++k) {
    int did = hits[k];
    vector<vector<int> > positions;
    vector<unsigned> upto;
    for (unsigned i = 0; i < phrase.size(); ++i) {
      int tid = phrase[i];
      ir->filldoc(tid);
      ir->fillpos(tid, did);
      vector<int>& v = ir->postings[tid][did];
      positions.push_back(v);
      //positions.push_back(vector<int>());
      //positions[i].insert(positions[i].begin(), v.begin(), v.end());
      upto.push_back(0);
    }
    vector<int> &pos0 = positions[0];
    while (upto[0] < pos0.size()) {
      success = true;
      for (unsigned i = 1; i < phrase.size(); ++i) {
        vector<int> &posi = positions[i];
        while (upto[i] < posi.size() &&
             posi[upto[i]] < pos0[upto[0]] + static_cast<int>(i)) {
          upto[i]++;
        }
        if (upto[i] == posi.size()) {
          success = false;
          break;
        }
        if (posi[upto[i]] != pos0[upto[0]] + static_cast<int>(i)) {
          success = false;
        }
      }
      if (success) {
        break;
      }
      upto[0]++;
    }
    if (success) {
      q->docs().push_back(did);
    }
  }
}

void IndexSearcher::searchNEAR(Query *q) {
  int offset = atoi(q->info.c_str());
  vector<int> hits;
  vector<int> phrase;

  disjunct(hits, q->get(0)->docs(), hits);
  conjunct(hits, q->get(1)->docs(), hits);

  for (unsigned i = 0; i < q->size(); ++i) {
    string t = q->get(i)->token;
    lowercase(t);
    porterstem(t);
    if (ir->termmap.find(t) == ir->termmap.end()) {
      return;
    }
    phrase.push_back(ir->termmap[t]);
  }
  for (unsigned i = 0; i < hits.size(); ++i) {
    ir->filldoc(phrase[0]);
    ir->filldoc(phrase[1]);
    ir->fillpos(phrase[0], hits[i]);
    ir->fillpos(phrase[1], hits[i]);
    vector<int> &v0 = ir->postings[phrase[0]][hits[i]];
    vector<int> &v1 = ir->postings[phrase[1]][hits[i]];
    unsigned s0 = 0, s1 = 0;
    int pos0, pos1, dist;
    bool success = false;
    while (s0 < v0.size() && s1 < v1.size()) {
      pos0 = v0[s0], pos1 = v1[s1];
      dist = pos0 - pos1;
      if (dist > 0)
        s1++;
      else
        s0++;
      dist = dist > 0 ? dist : -dist;
      if (dist <= offset) {
         success = true;
         break;
      }
    }
    if (success)
      q->docs().push_back(hits[i]);
  }
}


void IndexSearcher::report(Query* q) {
  cout << "query = " << q->tostring() << endl;
  cout << "numhit = " << q->docs().size() << endl;
  cout << "hitdocs = " << endl;
  for (unsigned i = 0; i < q->docs().size() && i < 10; ++i) {
    int did = q->docs()[i];
    cout<< "[" << did <<  "] " << ir->didmap[did] << endl;
  }
  if (q->docs().size() > 10) {
    cout << "... (result trucated)" << endl;
  }
}
