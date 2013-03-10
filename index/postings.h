#ifndef POSTINGS_H
#define POSTINGS_H

#include <vector>
#include <cstring>
using namespace std;

class PosEnum {
public:
    PosEnum(int);
    ~PosEnum();
    void add(int);
    int id();
    int next();
    int size();
    int offset();
private:
    int did;
    vector<int> positions;
    int upto;
};

class DocEnum {
public:
    DocEnum(int);
    ~DocEnum();
    PosEnum* next();
    void add(PosEnum*);
    int id();
    int size();
    int offset();
private:
    int tid;
    vector<PosEnum*> docs;
    int upto;
};

class TermEnum {
public:
    TermEnum();
    ~TermEnum();
    DocEnum* next();
    void add(DocEnum*);
    int size();
    int offset();
private:
    vector<DocEnum*> terms;
    int upto;
};
#endif
