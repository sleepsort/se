#ifndef POSTINGS_H
#define POSTINGS_H

#include <map>
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
    int at(int i);
    int end();
    int size();
    int offset();
    void reset();
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
    PosEnum* at(int i);
    void add(PosEnum*);
    int id();
    PosEnum* end();
    int size();
    int offset();
    void reset();
private:
    int tid;
    vector<PosEnum*> docs;
    int upto;
};

class TermEnum {
public:
    TermEnum();
    ~TermEnum();
    DocEnum* docs(int tid);
    DocEnum* next();
    DocEnum* end();
    void reset();
    void add(int key, DocEnum* de);
private:
    map<int, DocEnum*> terms;
    map<int, DocEnum*>::iterator iter;
};
#endif
