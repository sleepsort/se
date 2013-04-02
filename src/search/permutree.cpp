#include "search/permutree.h"
void PermuTree::search(string &token, vector<string> &collect) {
  //char buf[PERMU_BUF + 1];
  string rotated = token;
  rotate(rotated,'*');
  
}
void PermuTree::rotate(string &t, char d) {
  string s = t;
  int sz = t.length();
  if (t[sz - 1] == d) {
    t = s.substr(0, sz-1);
    return;
  }
  for (int i = 0; i < sz - 1; i++) {
    if (s[i] == d) {
      t = s.substr(i+1);
      t += s.substr(0, i);
    }
  }
}
