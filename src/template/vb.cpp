#include "template/vb.h"

// Different from vb, heading bit starts from 1, end with 0:
// 0:          00000000
// 824:        10111000 00000110
// 214577:     10110001 10001100 00001101 
// 4294967295: 11111111 11111111 11111111 11111111 00001111 
//
// T might be: unsigned int OR unsigned long long
//
template <class T>
void encode_vb(const T *raw, int num, char *data, int &size) {
  char *start = data;
  T m;
  for (int i = 0; i < num; i++) {
    m = raw[i];
    while (m > 0x7f) {
      *data++ = (m | 0x80);
      m >>= 7;              // logical shift for unsigned
    }
    *data++ = m;
  }
  size = data-start;
}

template <class T>
void decode_vb(const char *data, int size, T *raw, int &num) {
  const char *end= data + size;
  T n, tmp, *start = raw;
  int t;
  while (data < end) {
    n = 0, t = 0;
    while (*data & 0x80) {
      tmp = *data++ & 0x7f;  // in case T=long long
      n |= tmp << t;
      t += 7;
    }
    tmp = *data++ & 0x7f;
    n |= tmp << t;
    *raw++ = n;
  }
  num = raw - start;
}

template <class T>
void dgap(T *raw, int num) {
  T last = 0;
  for (int i = 0; i < num; i++) {
    raw[i] -= last;
    last = raw[i];
  }
}

template <class T>
void ungap(T *raw, int num) {
  T last = 0;
  for (int i = 0; i < num; i++) {
    raw[i] += last;
    last = raw[i];
  }
}
