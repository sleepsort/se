#include "query/parser.h"
Parser::Parser() {
  token = "";
  content = "";
  upto = 0;
  root = NULL;
}
Parser::~Parser() {
}

bool Parser::isoperator(char c) const {
  return (c == '(' || c == ')' ||
          c == '|' || c == '&' ||
          c == '!' || c == '\"'||
          c == '\\');
}
bool Parser::isalnumstar(char c) const {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') ||
          c == '*';
}
bool Parser::isoperator(const string& s) const {
  if (s.length() == 1)
    return isoperator(s[0]);
  return !(s.compare("OR")  && s.compare("AND") &&
           s.compare("NOT"));
}
bool Parser::isend() const {
  return token.length() == 0;
}

string Parser::next() {
  string &c = content;
  int cur, sz = c.size();

  while (upto < sz && isblank(c[upto]))
    upto++;

  if (upto >= sz)
    return "";

  if (upto < sz && isoperator(c[upto]))
    return c.substr(upto++, 1);

  cur = upto;
  while (upto < sz && isalnumstar(c[upto]))
    upto++;

  return c.substr(cur, upto-cur);
}
string Parser::peek() const {
  const string &c = content;
  int to = upto, cur, sz = c.size();

  while (to < sz && isblank(c[to]))
    to++;

  if (to >= sz)
    return "";

  if (to < sz && isoperator(c[to]))
    return c.substr(to++, 1);

  cur = to;
  while (to < sz && isalnumstar(c[to]))
    to++;

  return c.substr(cur, to-cur);
}


void Parser::match(const string &s) {
  if (!token.compare(s)) {
    token = next();
  } else {
    error("Parser::expect:'%s' but got: '%s'", s.c_str(),token.c_str());
    exit(1);
  }
}

Query* Parser::S() {
  Query* ret;
  if (!token.compare("(")) {  // (E)
    match("(");
    ret = E();
    match(")");
  } else if (!token.compare("NOT") || !token.compare("!")) {  // NOT *
    if (!token.compare("NOT"))
      match("NOT");
    else
      match("!");

    ret = new Query(SIGN_NOT);
    if (!token.compare("(")) {
      match("(");
      ret->add(E());
      match(")");
    } else {
      ret->add(new Query(token));
      token = next();
    }
  } else if (!token.compare("\"")) {  // " P "
    match("\"");
    ret = new Query(SIGN_PHRSE);
    while (token.compare("\"")) {
      ret->add(new Query(token));
      token = next();
    }
    match("\"");
  } else if (!peek().compare("\\")) { // W \N W
    ret = new Query(SIGN_NEAR);
    ret->add(new Query(token));
    token = next();
    match("\\");
    ret->info = token;
    ret->add(new Query(next()));
    token = next();
  } else {  // W
    size_t pos = token.find("*");  
    if (pos != string::npos ) {  // wildcard query
      if (token.find("*",pos+1) != string::npos) {
        token.erase(remove(token.begin()+pos+1, token.end(), '*'), token.end());
      }
      ret = new Query(SIGN_WILDCARD);
      ret->token = token;
    } else {  // normal single query
      ret = new Query(token);
    }
    token = next();
    if (!isoperator(token) && !isend()) {  // soft conjunction: A B
      Query* par = new Query(SIGN_OR);
      par->add(ret);
      while (!isoperator(token) && !isend()) {
        ret = new Query(token);
        ret->token = token;
        par->add(ret);
        token = next();
      }
      ret = par;
    }
  }
  return ret;
}

Query* Parser::T() {
  Query* par = new Query(SIGN_AND);
  par->add(S());
  while (!token.compare("AND") || !token.compare("&")) {  // S AND T
    if (!token.compare("AND"))
      match("AND");
    else
      match("&");
    par->add(S());
  }
  if (par->size() == 1) {
    Query* ret = par->get(0);
    par->clear();
    delete par;
    return ret;
  }
  return par;
}

Query* Parser::E() {
  Query* par = new Query(SIGN_OR);
  par->add(T());
  while (!token.compare("OR") || !token.compare("|")) {  // T OR E
    if (!token.compare("OR"))
      match("OR");
    else
      match("|");
    par->add(T());
  }
  if (par->size() == 1) {
    Query* ret = par->get(0);
    par->clear();
    delete par;
    return ret;
  }
  return par;
}

Query* Parser::parse(const string &str) {
  upto = 0;
  content = str;
  token = next();
  root = E();
  //root->token = str;
  return root;
}
