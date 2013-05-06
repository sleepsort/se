#include "index/writer.h"

const char IndexWriter::WORD_MAP_FILE[]  = "wmap.dat";
const char IndexWriter::TERM_MAP_FILE[]  = "tmap.dat";
const char IndexWriter::DOC_MAP_FILE[]   = "dmap.dat";
const char IndexWriter::GRAMS_FILE[]     = "grm.dat";
const char IndexWriter::PERMUTERM_FILE[] = "permut.dat";
const char IndexWriter::POSTINGS_FILE[]  = "pst.dat";
const int IndexWriter::MIN_N_GRAM = 2;
const int IndexWriter::MAX_N_GRAM = 2;

IndexWriter::IndexWriter(FileLoader &fl, const string &path) {
  this->fl = &fl;
  this->path = path;
  this->buf = new char[PST_BUF*10];
  this->didbuf = new unsigned[PST_BUF];
  this->frqbuf = new unsigned[PST_BUF];
}
IndexWriter::~IndexWriter() {
  delete []didbuf;
  delete []frqbuf;
  delete []buf;
}

void IndexWriter::write() {
  tick();
  writePST();
  tock();
}

void IndexWriter::flushWMAPBlk(const set<string> &wset, int turn) {
  string prefix = path+"/"+WORD_MAP_FILE;
  set<string>::iterator it;
  ofstream fwmap;

  fwmap.open((prefix+"."+itoa(turn)).c_str());
  for (it = wset.begin(); it != wset.end(); it++) {
    fwmap << *it << "\n";
  }
  fwmap.close();
}

// SPIMI-1: write current postings to intermediate files
//
//   {word}  =>  {ndoc => [doc][frq]}
// pst.trm.x       pst.doc.x         
//
// where x is the id of current intermediate file
//
void IndexWriter::flushPSTBlk(map<string, vector<pair<int, int> > >&pst, int turn) {
  ofstream ftrm, fdoc;
  string prefix = path+"/"+POSTINGS_FILE;

  ftrm.open((prefix+".trm."+itoa(turn)).c_str(), ios::binary);
  fdoc.open((prefix+".doc."+itoa(turn)).c_str(), ios::binary);

  map<string, vector<pair<int, int> > >::iterator it;
  vector<pair<int, int> >::iterator jt;

  for (it = pst.begin(); it != pst.end(); ++it) {
    string term = it->first;
    int ndoc = it->second.size(), cnt = 0, accum = 0;

    for (jt = it->second.begin(); jt != it->second.end(); ++jt) {
      int did = jt->first;
      int npos = jt->second;
      didbuf[cnt] = did;
      frqbuf[cnt] = npos;
      cnt++;
    }
    fwrite(fdoc, &ndoc, sizeof(ndoc));
    fwrite(fdoc, didbuf, sizeof(didbuf[0])*ndoc);
    fwrite(fdoc, frqbuf, sizeof(frqbuf[0])*ndoc);

    TermAttr attr;
    attr.str = term;
    attr.df = ndoc;
    attr.cf = accum;
    attr.flush(ftrm);
  }
  ftrm.close();
  fdoc.close();
}

// always pack from *.0 files (it must be pre-merged!)
void IndexWriter::packPSTBlk() {
  cout << "# packing postings..." << endl;
  ofstream pack_trm, pack_doc, pack_tmap;
  ifstream merge_trm, merge_doc;
  string prefix = path+"/"+POSTINGS_FILE, m1, m2;
  m1 = prefix+".trm";
  m2 = prefix+".doc";

  pack_trm.open(m1.c_str());
  pack_doc.open(m2.c_str(), ios::binary);
  pack_tmap.open((path+"/"+TERM_MAP_FILE).c_str(), ios::binary);

  merge_trm.open((m1+".0").c_str());
  merge_doc.open((m2+".0").c_str(), ios::binary);

  if (!merge_trm || !merge_doc) {
    error("Writer::fail opening %s, etc", (m1+".0").c_str());
  }
  int num_term = 0;
  TermAttr attr;
  while (attr.load(merge_trm)>=0) {
    int num_docs = 0, size;
    fread(merge_doc, &num_docs, sizeof(num_docs));
    fread(merge_doc, didbuf, sizeof(didbuf[0])*num_docs);
    fread(merge_doc, frqbuf, sizeof(frqbuf[0])*num_docs);

    pack_trm  << num_term << " " << ftellp(pack_doc) << "\n";
    assert(num_docs < PST_BUF);

    dgap(didbuf, num_docs);
    encode_vb(didbuf, num_docs, buf, size);
    assert(size < PST_BUF*10);
    fwrite(pack_doc, &size, sizeof(size));
    fwrite(pack_doc, buf, sizeof(buf[0])*size);

    encode_vb(frqbuf, num_docs, buf, size);
    assert(size < PST_BUF*10);
    fwrite(pack_doc, &size, sizeof(size));
    fwrite(pack_doc, buf, sizeof(buf[0])*size);

    attr.flush(pack_tmap);
    num_term++;
  }
  cout << "# num_term = " << num_term << endl;

  merge_trm.close();
  merge_doc.close();

  pack_trm.close();
  pack_doc.close();
  pack_tmap.close();

  remove((m1+".0").c_str());
  remove((m2+".0").c_str());
}
void IndexWriter::packWMAPBlk() {
  cout << "# packing wordmaps..." << endl;
  string prefix = path+"/"+WORD_MAP_FILE;
  ifstream merge_wmap;
  ofstream pack_wmap;
  merge_wmap.open((prefix+".0").c_str());
  pack_wmap.open(prefix.c_str());
  string word;
  int num_word = 0;
  while (merge_wmap >> word) {
    pack_wmap << num_word << " " << word << "\n";
    num_word++;
  }
  cout << "# num_word = " << num_word << endl;
  merge_wmap.close();
  pack_wmap.close();
  remove((prefix+".0").c_str());
}

void IndexWriter::mergeWMAPBlk(int numtmps, int headtmp, int destmp) {
  cout << "# merging wordmaps";
  cout << "[" << headtmp << "," << (headtmp+numtmps-1) << "]=>"<< destmp;
  cout << "..." << endl;
  string prefix = path+"/"+WORD_MAP_FILE;
  string merge_name = prefix+"."+itoa(destmp);
  ifstream tmp_files[numtmps];
  ofstream merge_wmap;

  merge_wmap.open((merge_name+".tmp").c_str());
  for (int i = 0, j = headtmp; i < numtmps; ++i, ++j) {
    tmp_files[i].open((prefix+"."+itoa(j)).c_str());
  }

  set<pair<string, int> > wordheap;
  for (int i = 0; i < numtmps; ++i) {
    ifstream& wmap = tmp_files[i];
    string word;
    if (wmap >> word) {
      wordheap.insert(make_pair(word, i));
    }
  }
  while (!wordheap.empty()) {
    set<pair<string, int> >::iterator it = wordheap.begin();
    pair<string, int> head = *it;

    merge_wmap << head.first << "\n";
    while (!wordheap.empty() && !it->first.compare(head.first)) {
      unsigned i = it->second;
      ifstream& wmap = tmp_files[i];
      string word;
      if (wmap >> word) {
        wordheap.insert(make_pair(word, i));
      }
      wordheap.erase(it);
      it = wordheap.begin();
    }
  }
  for (int i = 0, j = headtmp; i < numtmps; ++i, ++j) {
    tmp_files[i].close();
    remove((prefix+"."+itoa(j)).c_str());
  }
  merge_wmap.close();
  remove(merge_name.c_str());
  rename((merge_name+".tmp").c_str(), merge_name.c_str());
}

// SPIMI-2: K-way file merge.
//
// Each file is assumed to be around
// 400MB, thus quite possible to open those files at the same
// time.
//
// Every time, pickup the streams with minimum term,
// then read and merge postings from those files.
//
void IndexWriter::mergePSTBlk(int numtmps, int headtmp, int destmp) {
  cout << "# merging postings";
  cout << "[" << headtmp << "," << (headtmp+numtmps-1) << "]=>"<< destmp;
  cout << "..." << endl;
  ofstream merge_trm, merge_doc;
  ifstream tmp_files[numtmps*2];
  map<pair<string, int>, TermAttr> termheap;
  string prefix = path+"/"+POSTINGS_FILE, m1, m2;
  m1 = prefix+".trm."+itoa(destmp);
  m2 = prefix+".doc."+itoa(destmp);

  merge_trm.open((m1+".tmp").c_str());
  merge_doc.open((m2+".tmp").c_str(), ios::binary);

  for (int i = 0, j = headtmp; i < numtmps; ++i, ++j) {
    string s1, s2;
    s1 = prefix+".trm."+itoa(j);
    s2 = prefix+".doc."+itoa(j);
    tmp_files[i*2  ].open(s1.c_str(), ios::binary);
    tmp_files[i*2+1].open(s2.c_str(), ios::binary);
    if (!tmp_files[i*2] || !tmp_files[i*2+1]) {
      error("Writer::fail opening %s %s %s", s1.c_str(), s2.c_str(), "");
    }
  }
  for (int i = 0; i < numtmps; ++i) {
    ifstream& ftrm = tmp_files[i*2];
    TermAttr attr;
    if (attr.load(ftrm) >= 0) {
      string term = attr.str;
      termheap.insert(make_pair(make_pair(term, i), attr));
    }
  }
  int num_term = 0;
  while (!termheap.empty()) {
    map<pair<string, int>, TermAttr>::iterator it = termheap.begin();
    pair<string, int> head = it->first;
    set<int> hit;
    int num_docs = 0;

    while (!termheap.empty() && !it->first.first.compare(head.first)) {
      unsigned ndoc, i = it->first.second;
      ifstream& ftrm = tmp_files[i*2];
      ifstream& fdoc = tmp_files[i*2+1];

      hit.insert(i);

      TermAttr attr;
      if (attr.load(ftrm) >= 0) {
        string term = attr.str;
        termheap.insert(make_pair(make_pair(term, i), attr));
      }
      fpeek(fdoc, &ndoc, sizeof(ndoc));
      num_docs += ndoc;
      termheap.erase(it);
      it = termheap.begin();
    }

    int posupto = 0, didupto = 0;
    set<int>::iterator jt;
    for (jt = hit.begin(); jt != hit.end(); jt++) {
      unsigned i = *jt;
      ifstream& fdoc = tmp_files[i*2+1];

      int ndoc;
      fread(fdoc, &ndoc, sizeof(ndoc));
      fread(fdoc, &didbuf[didupto], sizeof(didbuf[0])*ndoc);
      fread(fdoc, &frqbuf[didupto], sizeof(frqbuf[0])*ndoc);
      didupto += ndoc;
    }
    assert(didupto == num_docs && didupto < PST_BUF);

    fwrite(merge_doc, &num_docs, sizeof(num_docs));
    fwrite(merge_doc, didbuf, sizeof(didbuf[0])*num_docs);
    fwrite(merge_doc, frqbuf, sizeof(frqbuf[0])*num_docs);

    TermAttr attr;
    attr.str = head.first;
    attr.df = didupto;
    attr.cf = posupto;
    attr.flush(merge_trm);

    num_term++;
  }
  for (int i = 0, j = headtmp; i < numtmps; ++i, ++j) {
    tmp_files[i*2 ].close();
    tmp_files[i*2+1].close();
    remove((prefix+".trm."+itoa(j)).c_str());
    remove((prefix+".doc."+itoa(j)).c_str());
  }
  merge_trm.close();
  merge_doc.close();
  remove(m1.c_str());
  remove(m2.c_str());
  rename((m1+".tmp").c_str(), m1.c_str());
  rename((m2+".tmp").c_str(), m2.c_str());
}


void IndexWriter::writePST() {
  ofstream fdmap;
  fdmap.open((path+"/"+DOC_MAP_FILE).c_str(), ios::binary);
  unsigned numtmps = 0;  // num of intermediate files
  unsigned numpsts = 0;  // num of position psts(to guess memory overhead)
  unsigned numfiles = 0;

  set<string> wordset;                            // {word}
  map<string, vector<pair<int, int> > > postings;  // {term => [did, tf]

  fl->init();
  while (fl->next()) { 
    if ((numfiles+1) % 10000 == 0) {
      cout << "(" << (numfiles+1) << ")" << endl;
    }
    vector<string> words;
    DocAttr attr;

    fl->attr(attr);
    fl->words(words);
    attr.flush(fdmap);

    int did = numfiles, n = words.size();
    for (int j = 0; j < n; ++j) {
      string t = words[j];
      lowercase(t);
      wordset.insert(t);
      porterstem(t);
      assert(t.length());
      vector<pair<int, int> > &v = postings[t];
      if (v.empty() || v[v.size()-1].first != did) {
        v.push_back(make_pair(did,1));
      } else {
        v[v.size()-1].second++;
      }
      numpsts++;
    }
    if (numpsts > 2e7) {
      flushPSTBlk(postings, numtmps);
      flushWMAPBlk(wordset, numtmps);
      numtmps++;
      numpsts = 0;
      postings.clear();
      wordset.clear();
    }
    numfiles++;
  }
  if (numpsts > 0) {
    flushPSTBlk(postings, numtmps);
    flushWMAPBlk(wordset, numtmps);
    numtmps++;
    numpsts = 0;
    postings.clear();
    wordset.clear();
  }
  fdmap.close();
  cout << "# numtmps=" << numtmps << endl;

  assert(MERGE_TMPS * 3 < 1024);  // limit of cocurrent opened files
  while (numtmps != 1) {
    unsigned cur = 0, newtmps = 0;
    while (cur + MERGE_TMPS < numtmps) {
      mergePSTBlk(MERGE_TMPS, cur, newtmps);
      mergeWMAPBlk(MERGE_TMPS, cur, newtmps);
      cur += MERGE_TMPS;
      newtmps++;
    }
    if (cur != numtmps) {
      mergePSTBlk(numtmps-cur, cur, newtmps);
      mergeWMAPBlk(numtmps-cur, cur, newtmps);
      newtmps++;
    }
    numtmps = newtmps;
  }
  packPSTBlk();
  packWMAPBlk();
}
