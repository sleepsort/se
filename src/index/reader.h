#ifndef INDEX_READER_H_
#define INDEX_READER_H_
#include "index/writer.h"
using namespace std;

// in-memory index
class IndexReader {
 public:
  IndexReader(string path);
  ~IndexReader();
  void read();

 public:
  map<int, string> didmap;   // did => doc filename
  map<int, string> tidmap;   // tid => term string
  map<int, string> widmap;   // wid => word string
  map<string, int> docmap;   // filename => did
  map<string, int> termmap;  // term => tid
  map<string, int> wordmap;  // word => wid

  map<string, vector<int> > grams;  // gram => [ wid ]

  map<int, map<int, vector<int> > > postings;  // tid => { did => [ pos ] }

 private:
  string path;
  bool loaded;
};

#endif  // INDEX_READER_H_
