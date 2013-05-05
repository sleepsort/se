#include "index/loader.h"
FileLoader::FileLoader(const string &index, const string &dir, const string type) {
  this->dir = string(dir);
  this->type = str2type(type);
  this->upto = 0;
  string info = index+"/type.ld";
  ofstream out;
  out.open(info.c_str());
  if (!out.is_open()) {
    error("FileLoader::fail open type file: %s", info.c_str()); 
  }
  out << dir << endl;
  out << type << endl;
  out.close();
}

FileLoader::FileLoader(const string &index) {
  string info = index+"/type.ld";
  ifstream in(info.c_str());
  if (!in.is_open()) {
    error("FileLoader::fail open type file: %s", info.c_str()); 
  }
  string type;
  getline(in, this->dir);
  getline(in, type);
  this->type = str2type(type);
  in.close();
}

FileLoader::~FileLoader() {
  if (fin.is_open()) {
    fin.close();
  }
}

string FileLoader::type2str(Corpus type) {
  switch (type) {
    case CORPUS_SHAKES:
      return "shakes";
    case CORPUS_RCV1:
      return "rcv1";
    case CORPUS_GOV2:
      return "gov2";
    default:
      return "raw";
  }
}
Corpus FileLoader::str2type(string str) {
  if (!str.compare("shakes")) {
    return CORPUS_SHAKES;
  } else if (!str.compare("rcv1")) {
    return CORPUS_RCV1;
  } else if (!str.compare("gov2")) {
    return CORPUS_GOV2;
  } else {
    return CORPUS_RAW;
  }
}

void FileLoader::init() {
  set<string> ex;
  if (type == CORPUS_SHAKES) {
    ex.insert("README");
    ex.insert("glossary");
  }
  collect(dir, files, ex);
}

void FileLoader::parse() {
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
}

void FileLoader::parseRCV1() {
  fin.close();
  string body = "";
  xmlbody(files[upto-1], body);
  tokenize(body.c_str(), m_words);
  m_content["body"] = body;
  m_content["name"] = files[upto-1];
  m_content["len"] = tostring(m_words.size());
  fin.open(files[0].c_str());         // just a hack to ensure we can continue
  fin.seekg(0, ios::end);
}
void FileLoader::parseGOV2() {
  char c[DOC_BUF + 10] = {0};
  char seps[] = "\036", flag = -1;
  char *p = c, *q = c;
  char *token, *cut;

  assert (fin.is_open());
  while (fin.getline(p, DOC_BUF, '\n')) {
    int l = strlen(p);
    if (l == 1 && p[0] == '\037' && flag == '\036') {
      p[0] = '\0';
      break;
    }
    if (l > 0) {
      flag = p[l-1];
    }
    p[l] = '\n'; p[l+1] = '\0';
    p += l+1;
  }
  p += strlen(p);
  while (q != p) {
    if (*q == '\036' && *(q+1) != '\n') {
      *q = '\035';
    }
    q++;
  }
  assert(strlen(c) < DOC_BUF - 10);
  //cout << "===" << c << "===" << endl;

  token = trim(strtok(c, seps));
  while (token && token[0]) {
    cut = strchr(token, '=');
    assert (*cut == '=');

    *cut++ = '\0';
    //cout << "---" << token << "|" << cut << "---" << endl;
    m_content[token] = cut;
    token = trim(strtok(NULL, seps));
  }
  string body = m_content["body"], title = m_content["title"];
  tokenize(body.c_str(), m_words);
  for (int i = 0; i < 5; i++) {    // title boost
    tokenize(title.c_str(), m_words);
  }
  body = title+"\n"+body; 
  m_content["body"] = body;
  m_content["name"] = m_content["trecid"];
  m_content["len"] = tostring(m_words.size());
}
void FileLoader::parseRAW() {
  char c[LINE_BUF + 10];
  string body = "";
  while (fin.getline(c, LINE_BUF, '\n')) {
    body += c; 
    tokenize(c, m_words);
  }
  m_content["body"] = body;
  m_content["name"] = files[upto-1];
  m_content["len"] = tostring(m_words.size());
}

bool FileLoader::next() {
  if (upto == files.size() && !fin.is_open()) {
    return false;
  } else if (!fin.is_open()) {
    fin.open(files[upto++].c_str());
  }
  //cout << files[upto-1] << endl;
  m_content.clear();
  m_words.clear();

  m_content["path"] = files[upto-1];
  m_content["offset"] = tostring(fin.tellg());

  parse();

  fin.peek();
  if (fin.eof()) {
    fin.close();
  }
  return true;
}


bool FileLoader::seek(const string &path, long long offset) {
  if (fin.is_open()) {
    fin.close();
  } else {
    fin.open(path.c_str());
    if (!fin) {
      return false;
    }
  }
  long long length;
  fin.seekg(0, ios::end);
  length = fin.tellg();
  if (length <= offset) {
    return false;
  }
  fin.seekg(offset, ios::beg);

  upto = 1;
  files.clear();
  files.push_back(path);
  parse();

  fin.close();
  return true;
}

void FileLoader::body(string &str) {
  assert( m_content.find("body") != m_content.end());
  str = m_content["body"];
}

void FileLoader::attr(DocAttr &attr) {
  attr.len = atoi(m_content["len"].c_str());
  attr.name = m_content["name"];
  attr.path = m_content["path"];
  attr.offset = atol(m_content["offset"].c_str());
  //cout << attr.path << " " << attr.name << " "  << attr.len << " " << attr.offset<<endl;
}

void FileLoader::words(vector<string> &words) {
  words.insert(words.begin(), m_words.begin(), m_words.end());
}

