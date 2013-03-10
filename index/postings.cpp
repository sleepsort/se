#include "postings.h"
TermEnum::TermEnum() {
    iter = terms.begin();
}
TermEnum::~TermEnum() {
    map<int, DocEnum*>::iterator it;
    for (it = terms.begin(); it != terms.end(); ++it) {
        delete it->second;
    }
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


void TermEnum::add(int tid, DocEnum* de) {
    terms.insert(make_pair(tid, de));
}
void DocEnum::add(PosEnum* pe) {
    docs.push_back(pe);
}
void PosEnum::add(int p) {
    positions.push_back(p);
}


DocEnum* TermEnum::docs(int tid) {
    map<int, DocEnum*>::iterator it = terms.find(tid);
    if (it == terms.end()) {
        return end();
    } else {
        return it->second;
    }
}

PosEnum* DocEnum::at(int off) {
    if (off > size()) {
        return end();
    } else {
        return docs[off];
    }
}
int PosEnum::at(int off) {
    if (off > size()) {
        return end();
    } else {
        return positions[off];
    }
}

DocEnum* TermEnum::next() {
    if (iter == terms.end()) {
        return end();
    } else {
        DocEnum* ret = iter->second;
        iter++;
        return ret;
    }
}
PosEnum* DocEnum::next() {
    if (upto > size()) {
        return end();
    } else {
        return docs[upto++];
    }
}
int PosEnum::next() {
    if (upto > size()) {
        return end();
    } else {
        return positions[upto++];
    }
}


DocEnum* TermEnum::end() {
    return NULL;
}
PosEnum* DocEnum::end() {
    return NULL;
}
int PosEnum::end() {
    return -1;
}

int DocEnum::offset() {
    return upto;
}
int PosEnum::offset() {
    return upto;
}


void TermEnum::reset() {
    iter = terms.begin();
}
void DocEnum::reset() {
    upto = 0;
}
void PosEnum::reset() {
    upto = 0;
}

int DocEnum::size() {
    return docs.size();
}
int PosEnum::size() {
    return positions.size();
}
