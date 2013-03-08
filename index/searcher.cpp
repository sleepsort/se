#include "searcher.h"
IndexSearcher::IndexSearcher(string path):IndexReader(path) {
    IndexReader::read();
    map<int, string>::iterator it;
    for (it = didmap.begin(); it != didmap.end(); ++it) {
        alldoc.push_back(it->first);
    }
}
IndexSearcher::~IndexSearcher() {
}
void IndexSearcher::search(Query *q) {
    if (q->sign == SIGN_NULL) {
        string t= q->token;
        int tid;
        transform(t.begin(), t.end(), t.begin(), ::tolower);
        if (termmap.find(t) != termmap.end()) {
            tid = termmap[t];
            vector<int> &v = postingslist[tid];
            q->docs().insert(q->docs().begin(), v.begin(), v.end());
        }
        return;
    }
    for (unsigned i=0; i<q->size(); i++) {
        search(q->get(i));
    }
    q->optimize();

    switch(q->sign) {
    case SIGN_NOT:
        diff(alldoc, q->get(0)->docs(), q->docs()); break;
    case SIGN_AND:
        disjunct(q->docs(), q->get(0)->docs(), q->docs());
        for (unsigned i=1; i<q->size(); i++) {
            conjunct(q->docs(), q->get(i)->docs(), q->docs());
        }
        break;
    case SIGN_OR:
        for (unsigned i=0; i<q->size(); i++) {
            disjunct(q->docs(), q->get(i)->docs(), q->docs());
        }
        break;
    default:
        break;
    }
}

void IndexSearcher::report(Query* q) {
    cout << "query = " << q->token << endl;
    cout << "numhit = " << q->docs().size() << endl;
    cout << "hitdocs = " << endl;
    for (unsigned i=0; i<q->docs().size(); i++) {
        int did = q->docs()[i];
        cout<< "[" << did <<  "] " << didmap[did] << endl;
    }
    cout << endl;
}

