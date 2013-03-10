#ifndef WRITER_H
#define WRITER_H
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include "util/util.h" 
using namespace std;

// write-once index,
// index structure cannot be updated or merged
class IndexWriter {
public:
    static const string TERM_MAP_FILE;
    static const string DOC_MAP_FILE;
    static const string POSTINGS_FILE;
    IndexWriter(string path);
    ~IndexWriter();
    void write(vector<string>& files);
    void flush();
private:
    string path;
    map<string, int> docmap;
    map<string, int> termmap;
    map<int, map<int, vector<int> > > postings;
    int numdocs;
    int numterms;
};

#endif
