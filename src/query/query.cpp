#include "query/query.h"
Query::Query(Sign s) {
  sign = s;
}
Query::Query(const string &t) {
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
vector<float>& Query::scores() {
  return hit_scores;
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

string Query::tostring() {
  string res = "";
  switch (sign) {
    case SIGN_SINGLE:
      return token;
    case SIGN_WILDCARD:
      return token;
    case SIGN_AND:
      res += "(";
      res += get(0)->tostring();
      for (unsigned i = 1; i < size(); ++i) {
        res += " & " + get(i)->tostring();
      }
      res += ")";
      break;
    case SIGN_OR:
      res += "(";
      res += get(0)->tostring();
      for (unsigned i = 1; i < size(); ++i) {
        res += " | " + get(i)->tostring();
      }
      res += ")";
      break;
    case SIGN_NOT:
      res = "! " + get(0)->tostring(); 
      break;
    case SIGN_PHRSE:
      res = "\""; 
        res += get(0)->tostring();
      for (unsigned i = 1; i < size(); ++i) {
        res += " " + get(i)->tostring();
      }
      res += "\"";
      break;
    case SIGN_NEAR:
      res += get(0)->tostring();
      res += " \\"+attr+" ";
      res += get(1)->tostring(); 
      break;
  }
  return res;
}
