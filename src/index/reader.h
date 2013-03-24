#ifndef INDEX_READER_H_
#define INDEX_READER_H_
#include <queue>
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
  
 public:
  void fillpostings(int tid, bool needpos);

 private:
  string path;
  map<string, long> docfp;   // term => fp in .pst.doc

  // value == tid, when a require of pst comes:
  // 1. pop front
  // 2. if front is required, simply push back and return
  // 3. if front is not required:
  //     erase it from pst_set
  //     if required exist in pst_set, return
  //     else push required to both pst_queue and pst_set
  queue<int> pst_queue; 
  map<int, bool> pst_pool;  // tid => needpos
};

#endif  // INDEX_READER_H_
