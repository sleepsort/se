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

/**
 * write-once index,
 * index structure cannot be updated or merged
 * 
 * did: document id
 * tid: term id, which is stemmed term
 * vid: vocabulary id, which is word, or pre-stemmed term
 *
 * TODO: no need to flush tid to *map_file, simply using line
 * number to indicate that!
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
  string path;

  // term dictionary ( mapping )
  map<int, string> didmap;  // did => doc filename 
  map<int, string> tidmap;  // tid => term string
  map<int, string> vidmap;  // vid => word string
  map<string, int> docmap;  // filename => did
  map<string, int> termmap; // term => tid
  map<string, int> wordmap; // word => vid

  map<string, vector<int> > grams; // k-gram index
  map<string, vector<int> > permutermlist; // un-structured permuterm

  // postings list, with position support
  map<int, map<int, vector<int> > > postings; // tid => { did => [ pos ] }

  int numdocs;
  int numterms;
  int numwords;
};

#endif
