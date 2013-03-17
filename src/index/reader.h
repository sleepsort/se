#ifndef READER_H
#define READER_H
#include "index/writer.h"
using namespace std;

// in-memory index
class IndexReader {
public:
  IndexReader(string path);
  ~IndexReader();
  void read();

public:
  map<int, string> didmap;  // did => doc filename 
  map<int, string> tidmap;  // tid => term string
  map<int, string> vidmap;  // vid => word string
  map<string, int> docmap;  // filename => did
  map<string, int> termmap; // term => tid
  map<string, int> wordmap; // word => vid

  map<string, vector<int> > grams; // gram => [ vid ]

  map<int, map<int, vector<int> > > postings; // tid => { did => [ pos ] }

private:
  string path;
  bool loaded;
};

#endif
