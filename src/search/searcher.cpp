#include "searcher.h"
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
    if (q->sign == SIGN_SINGLE) {
        searchSINGLE(q);
        return;
    }
    for (unsigned i = 0; i < q->size(); ++i)
        search(q->get(i));

    switch(q->sign) {
    case SIGN_NOT:
        searchNOT(q); break;
    case SIGN_AND:
        searchAND(q); break;
    case SIGN_OR:
        searchOR(q);  break;
    case SIGN_PHRSE:
        searchPHRSE(q); break;
    case SIGN_NEAR:
        searchNEAR(q); break;
    default:
        break;
    }
}

void IndexSearcher::searchSINGLE(Query *q) {
    string t= q->token;
    int tid;
    lowercase(t);
    porterstem(t);

    if (ir->termmap.find(t) != ir->termmap.end()) {
        tid = ir->termmap[t];
        map<int, map<int, vector<int> > >::iterator it; 
        map<int, vector<int> >::iterator jt; 
        it = ir->postings.find(tid); 
        if (it != ir->postings.end()) {
            vector<int> v;
            for (jt = it->second.begin(); jt!=it->second.end(); jt++) {
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
        string t= q->get(i)->token;
        int tid;
        lowercase(t);
        porterstem(t);
        if (ir->termmap.find(t) == ir->termmap.end()) {
            return;
        }
        tid = ir->termmap[t];
        phrase.push_back(tid);
    }
    for (unsigned k = 0; k < hits.size(); k++) {
        int did = hits[k];
        vector<vector<int>*> positions; 
        vector<unsigned> upto; 
        for (unsigned i = 0; i < phrase.size(); ++i) {
            int tid = phrase[i];
            positions.push_back(&(ir->postings[tid][did]));
            upto.push_back(0);
        }
        vector<int> &pos0 = (*positions[0]);
        while (upto[0] < pos0.size()) {
            success = true;
            for (unsigned i = 1; i < phrase.size(); ++i) {
                vector<int> &posi = (*positions[i]);
                while (upto[i] < posi.size() && 
                       posi[upto[i]] < pos0[upto[0]] + int(i)) {
                    upto[i]++;
                }
                if (upto[i] == posi.size()) {
                    success = false;
                    break;
                }
                if (posi[upto[i]] != pos0[upto[0]] + int(i)) {
                    success = false;
                }
            }
            if (success)
                break;
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
        string t= q->get(i)->token;
        lowercase(t);
        porterstem(t);
        if (ir->termmap.find(t) == ir->termmap.end()) {
            return;
        }
        phrase.push_back(ir->termmap[t]);
    }
    for (unsigned i = 0; i < hits.size(); ++i) {
        vector<int> &v0=ir->postings[phrase[0]][hits[i]];
        vector<int> &v1=ir->postings[phrase[1]][hits[i]];
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
cout << "query = " << q->token << endl;
cout << "numhit = " << q->docs().size() << endl;
    cout << "hitdocs = " << endl;
    for (unsigned i = 0; i < q->docs().size(); ++i) {
        int did = q->docs()[i];
        cout<< "[" << did <<  "] " << ir->didmap[did] << endl;
    }
    cout << endl;
}
