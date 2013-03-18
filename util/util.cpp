#include "util/util.h"


// should make sure sizeof(a) >= len + 1 !
// return: position we inserted the key
int array_insert(int* a, int len, int key) {
  int i = 0, j = 0;
  while (i < len && a[i] < key) {
    i++;
  }
  j = i;
  if (a[i] == key) {
    return -1;
  }
  while (j < len) {
    a[j+1] = a[j];
    j++;
  }
  a[i] = key;
  return i;
}
