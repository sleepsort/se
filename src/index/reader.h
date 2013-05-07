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
  long long ttf;

  vector<TermAttr> tidmap;   // tid => term info 
  vector<DocAttr>  didmap;   // did => doc info 

  map<int, string> widmap;   // wid => word string
  map<string, int> termmap;  // term => tid
  map<string, int> wordmap;  // word => wid

  map<string, vector<int> > grams;    // {gram => [ wid ]}
  map<int, map<int, int> > postings;  // {tid => { did => frq }}

  PermuTree permutree;       // B+ structured Permuterm tree
  
 public:
  void filldoc(int tid);
  void fillpos(int tid, int did);

 private:
  string path;

  // Term-doc relationship
  //map<int, long long> docfp;               // {tid => fp in .pst.doc}
  vector<long long> docfp;               // {tid => fp in .pst.doc}

  // Buffered postings info:
  // value == tid, when a require of pst comes:
  // 1. if tid is found in pool, change nothing
  // 2. else: pop queue, erase that one from pool, clear that pst list
  // 3.       load corresponding pst list, push into queue, and add to pool
  queue<int> pst_queue;                   // [tid]
  map<int, int> pst_pool;     // {tid => {did => needpos}}

 private:
  char *cbuf;
  unsigned *ibuf;
  unsigned *fbuf;
};

#endif  // INDEX_READER_H_
