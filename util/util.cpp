#include "util/util.h"

#include <iostream>
using namespace std;
// insert key to fit strict ascending order
int array_insert(int* a, int len, int key) {
  if (len == 0) {
    a[0] = key;
    return 0;
  }
  int i = 0, j = 0;
  /*
  while (i < len && a[i] < key) {
    i++;
  }
  j = i;
  */
  j = i = bsearch(a, len, key);
  if (i < len && a[i] == key) {  // duplicate
    return -1;
  }
  j = len;
  while (j > i) {
    a[j] = a[j-1];
    j--;
  }
  a[i] = key;
  return i;
}

// insert key in fixed position
// the array is unordered
void array_insert(int* a, int len, int key, int pos) {
  int j = len;
  if (a[pos] == key)
    return;
  while (j > pos) {
    a[j] = a[j-1];
    j--;
  }
  a[pos] = key;
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
  if (a[m] < key) {
    return m+1;
  } else {
    return m;
  }
}
