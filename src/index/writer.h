#ifndef INDEX_WRITER_H_
#define INDEX_WRITER_H_
#include <sys/stat.h>
#include <unistd.h>
#include <cassert>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include "util/debug.h"
#include "util/string.h"
#include "util/file.h"
#include "util/xml.h"
#include "search/permutree.h"
#include "template/compress.h"
using namespace std;

class TermAttr {
 public:
  string str;
  int df;
  int cf;
  int load(ifstream &is) {
    int str_len = 0, r = 0;
    char *tmp = NULL;
    if ((r = fread(is, &str_len, sizeof(int))) < 0) 
      return r;
    tmp = new char[str_len];
    fread(is, tmp, sizeof(char)*str_len);
    str = string(tmp);
    fread(is, &df, sizeof(int));
    fread(is, &cf, sizeof(int));
    delete tmp;
    return 0;
  }
  void flush(ofstream &os) {
    int str_len = str.length() + 1;
    fwrite(os, &str_len, sizeof(int));
    fwrite(os, str.c_str(), sizeof(char)*str_len);
    fwrite(os, &df, sizeof(int));
    fwrite(os, &cf, sizeof(int));
  } 
};

class DocAttr {
 public:
  string name;
  int len;
  int load(ifstream &is) {
    int name_len = 0, r = 0;
    char *tmp = NULL;
    if ((r = fread(is, &name_len, sizeof(int))) < 0)
      return r;
    tmp = new char[name_len];
    fread(is, tmp, sizeof(char)*name_len);
    name = string(tmp);
    fread(is, &len, sizeof(int));
    delete tmp;
    return 0;
  }
  void flush(ofstream &os) {
    int name_len = name.length() + 1;
    fwrite(os, &name_len, sizeof(int));
    fwrite(os, name.c_str(), sizeof(char)*name_len);
    fwrite(os, &len, sizeof(int));
  } 
};



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
  IndexWriter(const string &path);
  ~IndexWriter();

  void write(const vector<string>& files);

 private:
  void writePST(const vector<string>& files);
  void writeGRAMS();

  void flushPSTBlk(map<string, map<int, vector<int> > > &pst, int turn);
  void flushWMAPBlk(const set<string> &wset, int turn);

  void mergePSTBlk(int numtmps);
  void mergeWMAPBlk(int numtmps);


 private:
  string path;

 private:
  unsigned *didbuf, *posbuf;
  unsigned long long *fpbuf;
  char *buf;
};

#endif  // INDEX_WRITER_H_
