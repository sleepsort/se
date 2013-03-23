#include "util/util.h"

#include <iostream>
using namespace std;
// insert key in fixed position
int array_insert(int* a, int len, int key, int pos) {
  if (pos < len && a[pos] == key)
    return -1;
  int j = len;
  while (j > pos) {
    a[j] = a[j-1];
    j--;
  }
  a[pos] = key;
  return pos;
}

int bsearch(const int*a, int len, int key) {
  int f = 0, t = len -1, m = 0;
  while (f <= t) {
    m = (f+t)/2;
    if (a[m] == key) {
      return m;
    } else if (a[m] < key) {
      f = m+1;
    } else if (a[m] > key) {
      t = m-1;
    }
  }
  if (m < len && a[m] < key) {
    return m+1;
  } else {
    return m;
  }
}
