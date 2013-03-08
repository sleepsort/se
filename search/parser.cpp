#include "parser.h"
Parser::Parser() {
    token = "";
    content = "";
    upto = 0;
    root = NULL;
}
Parser::~Parser() {
}

string Parser::next_token() {
    string &c = content;
    int cur, sz = c.size();

    while (upto < sz && (c[upto] == ' ' || c[upto] == '\t'))
        upto++;

    if (upto >= sz)
        return "";

    if (upto < sz && (c[upto] == '(' || c[upto] == ')'))
        return c.substr(upto++,1);

    cur = upto;
    while (upto < sz && isalnum(c[upto]))
        upto++;

    return c.substr(cur, upto-cur);
}

void Parser::match(string s) {
    if (!token.compare(s)) {
        token = next_token();
    } else {
        cerr<<"expect:'"<<s<<"' but got:'"<<token<<"'"<<endl;
        exit(1);
    }
}

Query* Parser::S() {
    Query* ret;
    if (!token.compare("(")) {
        match("(");
        ret = E();
        match(")");
    } else if (!token.compare("NOT")) {
        match("NOT");
        ret = new Query(SIGN_NOT);
        if (!token.compare("(")) {
            match("(");
            ret->add(E());
            match(")");
        } else {
            ret->add(new Query(token));
            token = next_token();
        }
    } else {
        ret = new Query(token);
        token = next_token();
    }
    return ret;
}

Query* Parser::T() {
    Query* par = new Query(SIGN_AND);
    par->add(S());
    while (!token.compare("AND")) {
        match("AND");
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
    while (!token.compare("OR")) {
        match("OR");
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
    token = next_token();
    root = E();
    root->token = str;
    return root;
}
