#ifndef READER_H
#define READER_H
#include "writer.h"
using namespace std;

// in-memory index
class IndexReader {
public:
    IndexReader(string path);
    ~IndexReader();
    void read();

protected:
    string path;
    map<int, string> didmap;  // did => doc filename 
    map<int, string> tidmap;  // tid => term string
    map<string, int> docmap;  // filename => did
    map<string, int> termmap; // term => tid
    map<int, map<int, vector<int> > > postings;
};

#endif

