#include "util/string.h"

// simply make all character lowercased
void lowercase(string &t) {
  transform(t.begin(), t.end(), t.begin(), ::tolower);
}

// Using porter algorithm to stem word s
// input should make sure all words are lowercased
// TODO(billy): use snprintf instead of strcpy
void porterstem(string &s) {
  char t[WORD_BUF+10] = {0};
  strcpy(t, s.c_str());
  t[stem(t, 0, s.length()-1)+1] = '\0';
  s.replace(0, s.length(), t);
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
    while (value>0)
    {
        temp+=value%10+48;
        value/=10;
    }
    for (unsigned int i=0;i<temp.length();i++)
        returnvalue+=temp[temp.length()-i-1];
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

