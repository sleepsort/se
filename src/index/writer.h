#ifndef INDEX_WRITER_H_
#define INDEX_WRITER_H_
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include "util/util.h"
using namespace std;

/**
 * write-once index,
 * index structure cannot be updated or merged
 *
 * did: document id
 * tid: term id, which is stemmed term
 * wid: word id, which is pre-stemmed term
 *
 * TODO(billy): no need to flush tid to *map_file, 
 * simply using line number to indicate that!
 */
class IndexWriter {
 public:
  static const char WORD_MAP_FILE[];
  static const char TERM_MAP_FILE[];
  static const char DOC_MAP_FILE[];
  static const char GRAMS_FILE[];
  static const char PERMUTERM_FILE[];
  static const char POSTINGS_FILE[];
  static const int MIN_N_GRAM;
  static const int MAX_N_GRAM;
  IndexWriter(string path);
  ~IndexWriter();

  void write(const vector<string>& files);
  void flush();


 private:
  void writeDMAP(const vector<string>& files);
  void writePST(const vector<string>& files);
  void writeGRAMS();

  void flushPSTBlk(map<string, map<int, vector<int> > > &pst, int turn);
  void flushWMAPBlk(const set<string> &wset, int turn);

  void mergePSTBlk(int numtmps);
  void mergeWMAPBlk(int numtmps);

  void fread(ifstream &fin, void *buf, size_t len);
  void fpeek(ifstream &fin, void *buf, size_t len);
  void fwrite(ofstream &fout, void *buf, size_t len);

 private:
  string path;

  // term dictionary ( mapping )
  map<int, string> tidmap;   // tid => term string
  map<int, string> widmap;   // wid => word string
  map<string, int> termmap;  // term => tid
  map<string, int> wordmap;  // word => wid

  map<string, vector<int> > grams;  // k-gram index
  map<string, vector<int> > permutermlist;  // un-structured permuterm

  // postings list, with position support
};

#endif  // INDEX_WRITER_H_
