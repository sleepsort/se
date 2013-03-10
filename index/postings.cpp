#include "postings.h"
TermEnum::TermEnum() {
}
TermEnum::~TermEnum() {
    int sz = size();
    for (int i=0; i<sz; i++)
        delete terms[i];
}
DocEnum::DocEnum(int tid) {
    this->tid = tid;
    this->upto = 0;
}
DocEnum::~DocEnum() {
    int sz = size();
    for (int i=0; i<sz; i++)
        delete docs[i];
}
PosEnum::PosEnum(int did){ 
    this->did = did;
    this->upto = 0;
}
PosEnum::~PosEnum(){ 
}

int DocEnum::id() {
    return tid;
}
int PosEnum::id() {
    return did;
}


void TermEnum::add(DocEnum* te) {
    terms.push_back(te);
}
void DocEnum::add(PosEnum* de) {
    docs.push_back(de);
}
void PosEnum::add(int p) {
    positions.push_back(p);
}


DocEnum* TermEnum::next() {
    if (upto > size()) {
        return NULL;
    } else {
        return terms[upto++];
    }
}

PosEnum* DocEnum::next() {
    if (upto > size()) {
        return NULL;
    } else {
        return docs[upto++];
    }
}
int PosEnum::next() {
    if (upto > size()) {
        return -1;
    } else {
        return positions[upto++];
    }
}

int TermEnum::offset() {
    return upto;
}
int DocEnum::offset() {
    return upto;
}
int PosEnum::offset() {
    return upto;
}

int TermEnum::size() {
    return terms.size();
}
int DocEnum::size() {
    return docs.size();
}
int PosEnum::size() {
    return positions.size();
}
