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
  while (upto < sz && isalnum(c[upto]))
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
  while (to < sz && isalnum(c[to]))
    to++;

  return c.substr(cur, to-cur);
}


void Parser::match(string s) {
  if (!token.compare(s)) {
    token = next();
  } else {
    cerr << "Parser::expect:'" << s << "' but got:'" << token << "'" << endl;
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
    ret = new Query(token);
    token = next();
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

Query* Parser::parse(string str) {
  upto = 0;
  content = str;
  token = next();
  root = E();
  root->token = str;
  return root;
}
