#include "query.h"
Query::Query(Sign s) {
    sign = s;
}
Query::Query(string t) {
    sign = SIGN_SINGLE; 
    token = t;
}
Query::~Query() {
    for (unsigned i = 0; i < children.size(); ++i) 
        delete children[i];
}
void Query::add(Query* n) {
    children.push_back(n);
}
Query* Query::get(int i) {
    return children[i];
}
unsigned Query::size() {
    return children.size();
}
void Query::clear() {
    children.clear();
}
vector<int>& Query::docs() {
    return hit_docs;
}

typedef std::pair<Query *, int> q_pair;
bool sort_pred(const q_pair& l, const q_pair& r) {
    return l.second < r.second;
}
void Query::optimize() {
   if (size() < 2) 
       return;
   vector<q_pair> buff; 

   for (unsigned i = 0; i < size(); ++i) {
       buff.push_back(make_pair(get(i), get(i)->docs().size()));
   }
   sort(buff.begin(), buff.end(), sort_pred);
   clear();
   for (unsigned i = 0; i < buff.size(); ++i) {
       add(buff[i].first);
   }
}

void Query::dump() {
    switch(sign) {
        case SIGN_SINGLE:
            cout<<token<<" ";return;
        case SIGN_AND:
            cout<<"AND";break;
        case SIGN_OR:
            cout<<"OR";break;
        case SIGN_NOT:
            cout<<"NOT";break;
        case SIGN_PHRSE:
            cout<<"PHRSE";break;
        case SIGN_NEAR:
            cout<<"NEAR["<<info<<"]";break;
    }
    cout<<"("<<" ";
   for (unsigned i=0; i<size(); ++i) {
        get(i)->dump();
    }
    cout<<") ";
}
