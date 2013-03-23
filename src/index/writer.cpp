#include "index/writer.h"

const char IndexWriter::WORD_MAP_FILE[]  = "wmap.dat";
const char IndexWriter::TERM_MAP_FILE[]  = "tmap.dat";
const char IndexWriter::DOC_MAP_FILE[]   = "dmap.dat";
const char IndexWriter::GRAMS_FILE[]     = "grm.dat";
const char IndexWriter::PERMUTERM_FILE[] = "permut.dat";
const char IndexWriter::POSTINGS_FILE[]  = "pst.dat";
const int IndexWriter::MIN_N_GRAM = 2;
const int IndexWriter::MAX_N_GRAM = 2;

IndexWriter::IndexWriter(string path) {
  this->path = path;
  this->numdocs = 0;
  this->numterms = 0;
  this->numwords = 0;
}
IndexWriter::~IndexWriter() {
}
void IndexWriter::write(const vector<string>& files) {
  unsigned numfiles = files.size();
  for (unsigned i = 0; i < numfiles; ++i) {
    vector<string> words;
    int did = numdocs;
    unsigned n;

    docmap.insert(make_pair(files[i], did));
    didmap.insert(make_pair(did, files[i]));
    tokenize(files[i], words);
    n = words.size();

    for (unsigned j = 0; j < n; ++j) {
      map<string, int>::iterator it;
      string t = words[j];
      string w;
      int tid;

      lowercase(t);
      w = t;
      it = wordmap.find(w);
      if (it == wordmap.end()) {
        wordmap.insert(make_pair(w, numwords));
        widmap.insert(make_pair(numwords, w));
        numwords++;
      }
      porterstem(t);
      if ((it = termmap.find(t)) != termmap.end()) {
        tid = it->second;
      } else {
        termmap.insert(make_pair(t, numterms));
        tidmap.insert(make_pair(numterms, t));
        tid = numterms++;
      }

      map<int, map<int, vector<int> > >::iterator jt;
      jt = postings.find(tid);
      if (jt == postings.end()) {
        postings.insert(make_pair(tid, map<int, vector<int> >()));
        jt = postings.find(tid);
      }

      map<int, vector<int> >::iterator kt;
      kt = jt->second.find(did);
      if (kt == jt->second.end()) {
        jt->second.insert(make_pair(did, vector<int>()));
        kt = jt->second.find(did);
      }
      kt->second.push_back(j);
    }
    numdocs++;
  }

  map<int, string>::iterator it;
  for (it = widmap.begin(); it != widmap.end(); ++it) {
    int wid = it->first;
    string w = it->second;
    int sz = w.length();
    // k-gram
    for (int k = MIN_N_GRAM; k <= MAX_N_GRAM; ++k) {
      for (int n = 0; n < sz - 1; ++n) {
        string g = w.substr(n, k);
        map<string, vector<int> >::iterator jt;
        jt = grams.find(g);
        if (jt == grams.end()) {
          grams.insert(make_pair(g, vector<int>()));
          jt = grams.find(g);
        }
        vector<int>& v = jt->second;
        if (v.empty() || *(v.rbegin()) != wid) {
          v.push_back(wid);
        }
      }
    }
    // permuterm, only store words without exact match
    w = w+"$"+w;
    for (int n = 0; n < sz; ++n) {
      string term = w.substr(n, sz + 1);
      permutermlist[term].push_back(wid);
    }
  }
}
void IndexWriter::flush() {
  rmdir(path.c_str());
  int status = mkdir(path.c_str(), S_IRWXU);
  if (status != 0) {
    cerr << "Writer::fail create index directory: " << path << endl;
    return;
  }
  map<int, string>::iterator it;
  map<int, map<int, vector<int> > >::iterator jt;
  map<int, vector<int> >::iterator kt;
  map<string, vector<int> >::iterator lt;

  ofstream fout;

  fout.open((path+"/"+WORD_MAP_FILE).c_str());
  for (it = widmap.begin(); it != widmap.end(); ++it) {
    fout << it->first << " " << it->second << endl;
  }
  fout.close();

  fout.open((path+"/"+TERM_MAP_FILE).c_str());
  for (it = tidmap.begin(); it != tidmap.end(); ++it) {
    fout << it->first << " " << it->second << endl;
  }
  fout.close();

  fout.open((path+"/"+DOC_MAP_FILE).c_str());
  for (it = didmap.begin(); it != didmap.end(); ++it) {
    fout << it->first << " " << it->second << endl;
  }
  fout.close();

  fout.open((path+"/"+POSTINGS_FILE).c_str());
  for (jt = postings.begin(); jt != postings.end(); ++jt) {
    fout << jt->first << " " << jt->second.size() << endl;
    for (kt = jt->second.begin(); kt != jt->second.end(); ++kt) {
      fout << " " << kt->first;
      fout << " " << kt->second.size() << endl << "  ";
      for (unsigned i = 0; i < kt->second.size(); ++i) {
        fout << kt->second[i] << " ";
      }
      fout << endl;
    }
  }
  fout.close();

  fout.open((path+"/"+GRAMS_FILE).c_str());
  for (lt = grams.begin(); lt != grams.end(); ++lt) {
    fout << lt->first << " " << lt->second.size() << endl;
    vector<int> &v = lt->second;
    for (unsigned i = 0; i < v.size(); ++i) {
      fout << " " << v[i];
    }
    fout << endl;
  }
  fout.close();
}
