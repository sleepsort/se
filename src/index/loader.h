#ifndef INDEX_LOADER_H_
#define INDEX_LOADER_H_
#include <sys/stat.h>
#include <cassert>
#include <fstream>
#include "index/writer.h"
#include "util/string.h"
#include "util/file.h"
#include "util/xml.h"
using namespace std;

class DocAttr;

enum Corpus {
    CORPUS_SHAKES,
    CORPUS_RCV1,
    CORPUS_GOV2,
    CORPUS_RAW
};

class FileLoader {
 public:
  FileLoader(const string &dir, Corpus type = CORPUS_RAW);
  ~FileLoader();

  void init();

  bool next();
  bool seek(const string &path, long long offset);

  void attr(DocAttr &attr);
  void body(string &str);
  void words(vector<string> &words);

 private:
  void parse();
  void parseRCV1();
  void parseGOV2();
  void parseRAW();


 private:
  string dir;
  Corpus type;

  ifstream fin;
  vector<string> files;
  unsigned upto;

  vector<string> m_words;
  map<string, string> m_content;
};

#endif  // INDEX_LOADER_H_
