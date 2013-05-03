#include "index/loader.h"
FileLoader::FileLoader(const string &dir, Corpus type) {
  this->dir = string(dir);
  this->type = type;
  this->upto = 0;
}
void FileLoader::init() {
  set<string> ex;
  if (type == CORPUS_SHAKES) {
    ex.insert("README");
    ex.insert("glossary");
  }
  collect(dir, files, ex);
}

void FileLoader::parseRCV1() {
  fin.close();
  xmltokenize(files[upto-1], m_words);
  m_content["name"] = files[upto-1];
  m_content["len"] = tostring(m_words.size());
  fin.open(files[0].c_str());         // just a hack to ensure we can continue
  fin.seekg(ios_base::end);
}
void FileLoader::parseGOV2() {

}
void FileLoader::parseRAW() {
  char c[LINE_BUF + 10];
  while (fin.getline(c, LINE_BUF, '\n')) {
    tokenize(c, m_words);
  }
  m_content["name"] = files[upto-1];
  m_content["len"] = tostring(m_words.size());
}

bool FileLoader::next() {
  if (upto == files.size() && !fin.is_open()) {
    return false;
  } else if (!fin.is_open()) {
    fin.open(files[upto++].c_str());
  }
  m_content.clear();
  m_words.clear();

  m_content["path"] = files[upto-1];
  m_content["offset"] = tostring(fin.tellg());

  switch (type) {
    case CORPUS_RCV1:
      parseRCV1();
      break;
    case CORPUS_GOV2:
      parseGOV2();
      break;
    case CORPUS_SHAKES:
    case CORPUS_RAW:
      parseRAW();
      break;
  }
  fin.peek();
  if (fin.eof()) {
    fin.close();
  }
  return true;
}

void FileLoader::attr(DocAttr &attr) {
  attr.len = atoi(m_content["len"].c_str());
  attr.name = m_content["name"];
  //attr.path = m_content["path"];
  //attr.offset = atol(m_content["offset"].c_str());
}

void FileLoader::words(vector<string> &words) {
  words.insert(words.begin(), m_words.begin(), m_words.end());
}

FileLoader::~FileLoader() {
  if (fin.is_open()) {
    fin.close();
  }
}
