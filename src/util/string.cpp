#include "util/string.h"

void tokenize(const char *str, vector<string> &collect) {
  if (!str || !str[0]) {
    return;
  }
  char c[LINE_BUF+10];
  char *end, *upto, *cur;

  strcpy(c, str);
  upto = c;
  end = c + strlen(c);

  while (upto < end) {
    //while (upto < end && !isalpha(*upto))
    while (upto < end && !isalnum(*upto))
      upto++;
    if (upto >= end)
      break;
    cur = upto;
    //while (upto < end && isalpha(*upto))
    while (upto < end && isalnum(*upto))
      upto++;
    *upto = '\0';
    if (upto - cur < WORD_BUF && nodigit(cur)) {
      collect.push_back(cur);
    }
  }
}

bool nodigit(const char *c) {
  if (!c || !c[0]) {
    return false;
  }
  while (*c) {
    if (*c <= '9' && *c >= '0') {
      return false;
    }
    c++;
  }
  return true;
}

// simply make all character lowercased
void lowercase(string &t) {
  transform(t.begin(), t.end(), t.begin(), ::tolower);
}

// Using porter algorithm to stem word s
// input should make sure all words are lowercased
// TODO(billy): use snprintf instead of strcpy
void porterstem(string &s) {
  char t[WORD_BUF+10] = {0};
  int sz = s.length();
  strcpy(t, s.c_str());
  t[stem(t, 0, sz-1)+1] = '\0';
  s.replace(0, sz, t);
}


/*
 * String style "itoa":
 * Written by Oromis
 */
string itoa(int value) {
  if (value == 0)
    return "0";
  std::string temp="";
  std::string returnvalue="";
  while (value>0) {
    temp+=value%10+48;
    value/=10;
  }
  for (unsigned int i = 0; i < temp.length(); ++i) {
    returnvalue+=temp[temp.length()-i-1];
  }
  return returnvalue;
}

int min(int a1, int a2, int a3) {
  if (a1 < a2) {
    return a1 < a3 ? a1 : a3;
  } else {
    return a2 < a3 ? a2 : a3;
  }
}

// edit distance between two strings
// it is assumed that add,delete,replace share the same weight
int levendistance(const string &s1, const string &s2) {
  int d[WORD_BUF][WORD_BUF] = { {0} };
  int sz1 = s1.length(), sz2 = s2.length();
  if (sz1 == 0 || sz2 == 0)
    return -1;
  for (int i = 0; i <= sz1; ++i)
    d[i][0] = i;
  for (int j = 0; j <= sz2; ++j)
    d[0][j] = j;
  for (int j = 1; j <= sz2; ++j) {
    for (int i = 1; i <= sz1; ++i) {
      if (s1[i-1] == s2[j-1]) {
        d[i][j] = d[i-1][j-1];
      } else {
        d[i][j] = min(d[i-1][j], d[i][j-1], d[i-1][j-1]) + 1;
      }
    }
  }
  return d[sz1][sz2];
}

/**
 * Delete the heading and trailing ' ','\n',etc,
 * which are defined as isspace();
 *
 * Return: 
 *  string trimed 
 */
char *trim(char *str) {
  char *ibuf = str, *obuf = str;
  int i = 0, cnt = 0;
  if (str) {
    //  Remove leading spaces (from RMLEAD.C)
    for (ibuf = str; *ibuf && isspace(*ibuf); ++ibuf)
      ;
    if (str != ibuf) {
      memmove(str, ibuf, ibuf - str);
    }
    //  Collapse embedded spaces (from LV1WS.C)
    while (*ibuf) {
      if (isspace(*ibuf) && cnt) {
        ibuf++;
      } else {
        if (!isspace(*ibuf)) {
          cnt = 0;
        } else {
          *ibuf = ' ';
          cnt = 1;
        }
        obuf[i++] = *ibuf++;
      }
    }
    obuf[i] = '\0';
    //  Remove trailing spaces (from RMTRAIL.C)
    while (--i >= 0) {
      if (!isspace(obuf[i])) {
        break;
      }
    }
    obuf[++i] = '\0';
  }
  return str;
}

const char *byte2bin(char x) {
  static char b[9];
  b[0] = '\0';
  int z;
  for (z = 1<<7; z > 0; z >>= 1)
    strcat(b, ((x & z) == z) ? "1" : "0");
  return b;
}

