#include "suggester.h"
Suggester::Suggester(IndexReader &r) {
    this->ir = &r;
    this->ir->read();
}
Suggester::~Suggester() {
}
