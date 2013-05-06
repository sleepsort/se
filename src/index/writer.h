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
#include "index/loader.h"
#include "search/permutree.h"
#include "template/compress.h"
using namespace std;

class FileLoader;

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
  string path;
  long long offset;
  int len;
  int load(ifstream &is) {
    int name_len = 0, path_len = 0, r = 0;
    char tmp[PATH_BUF];
    if ((r = fread(is, &name_len, sizeof(int))) < 0)
      return r;
    fread(is, tmp, sizeof(char)*name_len);
    name = string(tmp);
    if ((r = fread(is, &path_len, sizeof(int))) < 0)
      return r;
    fread(is, tmp, sizeof(char)*path_len);
    path = string(tmp);
    fread(is, &offset, sizeof(long long));
    fread(is, &len, sizeof(int));
    return 0;
  }
  void flush(ofstream &os) {
    int name_len = name.length() + 1;
    int path_len = path.length() + 1;
    fwrite(os, &name_len, sizeof(int));
    fwrite(os, name.c_str(), sizeof(char)*name_len);
    fwrite(os, &path_len, sizeof(int));
    fwrite(os, path.c_str(), sizeof(char)*path_len);
    fwrite(os, &offset, sizeof(long long));
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
  IndexWriter(FileLoader &fl, const string &path);
  ~IndexWriter();

  void write();

 private:
  void writePST();
  void writeGRAMS();

  void flushPSTBlk(map<string, vector<pair<int, int> > > &pst, int turn);
  void flushWMAPBlk(const set<string> &wset, int turn);

  void packPSTBlk();
  void packWMAPBlk();

  void mergePSTBlk(int numtmps, int headtmp, int destmp);
  void mergeWMAPBlk(int numtmps, int headtmp, int destmp);


 private:
  FileLoader *fl;
  string path;

 private:
  unsigned *didbuf, *frqbuf;
  char *buf;
};

#endif  // INDEX_WRITER_H_
