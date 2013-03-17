#ifndef QUERY_PARSER_H_
#define QUERY_PARSER_H_

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "query/query.h"

using namespace std;

/**
 * Grammar:
 *
 * E ::= T OR E   |
 *       T | E    |
 *       T
 * T ::= S AND T  |
 *       S & T    |
 *       S
 * S ::= NOT ( E ) |
 *       ! ( E )   |
 *       ( E )     |
 *       " P "     |
 *       W \N W    |
 *       NOT W     |
 *       ! W       |
 *       W
 * P ::= W W*
 * W ::= [A-Za-z0-9]+
 * N ::= [0-9]+
 *
 */
class Parser {
 public:
  Parser();
  ~Parser();
  Query * parse(string str);

 private:
  bool isoperator(char c) const;

  Query* E();
  Query* T();
  Query* S();
  Query* P();

  string next();
  string peek() const;

  void match(string str);

 private:
  string content;
  string token;
  int upto;
  Query* root;
};

#endif  // QUERY_PARSER_H_
