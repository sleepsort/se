#include "index/reader.h"
IndexReader::IndexReader(string path) {
  this->path = path;
  this->loaded = false;  // TODO(billy): shit, this var is useless
  read();
}
IndexReader::~IndexReader() {
}
void IndexReader::read() {
  if (loaded) {
    return;
  }
  ifstream fin;
  string token;
  int vid, tid, did;
  map<int, map<int, vector<int> > >::iterator it;
  map<int, vector<int> >::iterator jt;
  map<string, vector<int> >::iterator kt;

  fin.open((path+"/"+IndexWriter::WORD_MAP_FILE).c_str());
  while (fin >> vid) {
    fin.ignore();
    getline(fin, token);
    wordmap.insert(make_pair(token, vid));
    vidmap.insert(make_pair(vid, token));
  }
  fin.close();

  fin.open((path+"/"+IndexWriter::TERM_MAP_FILE).c_str());
  while (fin >> tid) {
    fin.ignore();
    getline(fin, token);
    termmap.insert(make_pair(token, tid));
    tidmap.insert(make_pair(tid, token));
  }
  fin.close();

  fin.open((path+"/"+IndexWriter::DOC_MAP_FILE).c_str());
  while (fin >> did) {
    fin.ignore();
    getline(fin, token);
    docmap.insert(make_pair(token, did));
    didmap.insert(make_pair(did, token));
  }
  fin.close();

  fin.open((path+"/"+IndexWriter::POSTINGS_FILE).c_str());
  while (fin >> tid) {
    int n, m, pos;
    fin >> n;
    postings.insert(make_pair(tid, map<int, vector<int> >()));
    it = postings.find(tid);
    for (int i = 0; i < n; ++i) {
      fin >> did >> m;
      it->second.insert(make_pair(did, vector<int>()));
      jt = it->second.find(did);
      for (int j = 0; j < m; ++j) {
        fin >> pos;
        jt->second.push_back(pos);
      }
    }
  }
  fin.close();

  fin.open((path+"/"+IndexWriter::GRAMS_FILE).c_str());
  while (getline(fin, token, ' ')) {
    int n;
    fin >> n;
    grams.insert(make_pair(token, vector<int>()));
    kt = grams.find(token);
    for (int i = 0; i < n; ++i) {
      fin >> vid;
      kt->second.push_back(vid);
    }
    fin.ignore();
  }
  fin.close();

  loaded = true;
}
