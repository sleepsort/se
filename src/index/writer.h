#ifndef INDEX_WRITER_H_
#define INDEX_WRITER_H_
#include <sys/stat.h>
#include <unistd.h>
#include <cassert>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include "util/string.h"
#include "util/file.h"
#include "util/xml.h"
using namespace std;

/**
 * write-once index,
 * index structure cannot be updated or merged
 *
 * did: document id
 * tid: term id, which is stemmed term
 * wid: word id, which is pre-stemmed term
 *
 * TODO(billy): no need to flush id to *map_file, 
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

 private:
  void writeDMAP(const vector<string>& files);
  void writePST(const vector<string>& files);
  void writeGRAMS();

  void flushPSTBlk(map<string, map<int, vector<int> > > &pst, int turn);
  void flushWMAPBlk(const set<string> &wset, int turn);

  void mergePSTBlk(int numtmps);
  void mergeWMAPBlk(int numtmps);


 private:
  string path;
};

#endif  // INDEX_WRITER_H_
