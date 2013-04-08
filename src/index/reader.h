#ifndef INDEX_READER_H_
#define INDEX_READER_H_
#include <malloc.h>
#include <queue>
#include "index/writer.h"
using namespace std;

// in-memory index
class IndexReader {
 public:
  IndexReader(const string &path);
  ~IndexReader();
  void read();

 public:
  map<int, TermAttr> tidmap;   // tid => term info 
  map<int, DocAttr>  didmap;   // did => doc info 

  map<int, string> widmap;   // wid => word string
  map<string, int> termmap;  // term => tid
  map<string, int> wordmap;  // word => wid

  map<string, vector<int> > grams;  // {gram => [ wid ]}
  map<int, map<int, vector<int> > > postings;  // {tid => { did => [ pos ] }}

  PermuTree permutree;       // B+ structured Permuterm tree
  
 public:
  void filldoc(int tid);
  void fillpos(int tid, int did);

 private:
  string path;

  // Term-doc relationship
  map<int, long long> docfp;               // {tid => fp in .pst.doc}

  // Buffered postings info:
  // value == tid, when a require of pst comes:
  // 1. if tid is found in pool, change nothing
  // 2. else: pop queue, erase that one from pool, clear that pst list
  // 3.       load corresponding pst list, push into queue, and add to pool
  queue<int> pst_queue;                   // [tid]
  map<int, map<int, bool> > pst_pool;     // {tid => {did => needpos}}
  map<int, map<int, long long> > posfp;   // {tid => {did => fp in .pst.pos} }

 private:
  char *cbuf;
  unsigned *ibuf;
  unsigned long long *lbuf;
};

#endif  // INDEX_READER_H_
