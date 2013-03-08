#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "query.h"

using namespace std;

/**
 * Grammar: 
 *
 * E -> T OR E | T
 * T -> S AND T | S
 * S-> NOT (E) | (E) | NOT [A-Za-z0-9]+ | [A-Za-z0-9]+
 *
 */
class Parser {
public:
    Parser();
    ~Parser();
    Query * parse(string str);

private:
    Query* E();
    Query* T();
    Query* S();
    string next_token();
    void match(string str);

private:
    string content;
    string token; 
    int upto;
    Query* root;
};

#endif
