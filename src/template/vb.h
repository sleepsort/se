#ifndef TEMPLATE_VB_H_
#define TEMPLATE_VB_H_


template <class T>
void encode_vb(const T *raw, int num, char *data, int &size);

template <class T>
void decode_vb(const char *data, int size, T *raw, int &num);

#include "template/vb.cpp"
#endif  // TEMPLATE_VB_H_
