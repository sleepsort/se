#ifndef SUGGESTER_H
#define SUGGESTER_H
#include "index/reader.h"
class Suggester {
public:
    Suggester(IndexReader &r);
    ~Suggester();

private:
    IndexReader *ir;
};

#endif 
