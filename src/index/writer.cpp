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
  this->posbuf = new unsigned[PST_BUF];
  this->fpbuf = new unsigned long long[PST_BUF];
}
IndexWriter::~IndexWriter() {
  delete []didbuf;
  delete []posbuf;
  delete []fpbuf;
  delete []buf;
}

void IndexWriter::write() {
  tick();
  writePST();
  tock();
  tick();
  writeGRAMS();
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
//   {word}  =>  {ndoc => [doc]}  =>  {npos => [pos]}
// pst.trm.x       pst.doc.x            pst.pos.x
//
// where x is the id of current intermediate file
//
void IndexWriter::flushPSTBlk(map<string, map<int, vector<int> > > &pst, int turn) {
  ofstream ftrm, fdoc, fpos;
  string prefix = path+"/"+POSTINGS_FILE;

  ftrm.open((prefix+".trm."+itoa(turn)).c_str(), ios::binary);
  fdoc.open((prefix+".doc."+itoa(turn)).c_str(), ios::binary);
  fpos.open((prefix+".pos."+itoa(turn)).c_str(), ios::binary);

  map<string, map<int, vector<int> > >::iterator it;
  map<int, vector<int> >::iterator jt;

  for (it = pst.begin(); it != pst.end(); ++it) {
    string term = it->first;
    int ndoc = it->second.size(), cnt = 0, accum = 0;

    for (jt = it->second.begin(); jt != it->second.end(); ++jt) {
      int did = jt->first;
      int npos = jt->second.size();

      copy(jt->second.begin(), jt->second.begin()+npos, posbuf);
      fwrite(fpos, &npos, sizeof(npos));
      fwrite(fpos, posbuf, sizeof(posbuf[0]) * npos);
      accum += npos;

      didbuf[cnt] = did;
      cnt++;
    }
    fwrite(fdoc, &ndoc, sizeof(ndoc));
    fwrite(fdoc, didbuf, sizeof(didbuf[0])*ndoc);

    TermAttr attr;
    attr.str = term;
    attr.df = ndoc;
    attr.cf = accum;
    attr.flush(ftrm);
  }
  ftrm.close();
  fdoc.close();
  fpos.close();
}

// always pack from *.0 files (it must be pre-merged!)
void IndexWriter::packPSTBlk() {
  cout << "# packing postings..." << endl;
  ofstream pack_trm, pack_doc, pack_pos, pack_tmap;
  ifstream merge_trm, merge_doc, merge_pos;
  string prefix = path+"/"+POSTINGS_FILE, m1, m2, m3;
  m1 = prefix+".trm";
  m2 = prefix+".doc";
  m3 = prefix+".pos";

  pack_trm.open(m1.c_str());
  pack_doc.open(m2.c_str(), ios::binary);
  pack_pos.open(m3.c_str(), ios::binary);
  pack_tmap.open((path+"/"+TERM_MAP_FILE).c_str(), ios::binary);

  merge_trm.open((m1+".0").c_str());
  merge_doc.open((m2+".0").c_str(), ios::binary);
  merge_pos.open((m3+".0").c_str(), ios::binary);

  if (!merge_trm || !merge_doc || !merge_pos) {
    error("Writer::fail opening %s, etc", (m1+".0").c_str());
  }
  int num_term = 0;
  TermAttr attr;
  while (attr.load(merge_trm)>=0) {
    int num_docs = 0, tot_pos = 0, size;
    fread(merge_doc, &num_docs, sizeof(num_docs));
    fread(merge_doc, didbuf, sizeof(didbuf[0])*num_docs);

    pack_trm  << num_term << " " << ftellp(pack_doc) << "\n";
    for (int i = 0; i < num_docs; i++) {
      int num_pos = 0;
      fpbuf[i] = ftellp(pack_pos);

      fread(merge_pos, &num_pos, sizeof(num_pos));
      fread(merge_pos, posbuf, sizeof(posbuf[0])*num_pos);

      assert(num_pos > 0 && num_pos < PST_BUF);
      tot_pos += num_pos;
  
      dgap(posbuf, num_pos);
      encode_vb(posbuf, num_pos, buf, size);
      assert(size < PST_BUF*2);
      fwrite(pack_pos, &size, sizeof(size));
      fwrite(pack_pos, buf, sizeof(buf[0])*size);
    }
    assert(num_docs < PST_BUF);
    assert(attr.cf == tot_pos);

    dgap(didbuf, num_docs);
    encode_vb(didbuf, num_docs, buf, size);
    assert(size < PST_BUF*10);
    fwrite(pack_doc, &size, sizeof(size));
    fwrite(pack_doc, buf, sizeof(buf[0])*size);

    dgap(fpbuf, num_docs);
    encode_vb(fpbuf, num_docs, buf, size);
    assert(size < PST_BUF*10);
    fwrite(pack_doc, &size, sizeof(size));
    fwrite(pack_doc, buf, sizeof(buf[0])*size);

    attr.flush(pack_tmap);
    num_term++;
  }
  cout << "# num_term = " << num_term << endl;

  merge_trm.close();
  merge_doc.close();
  merge_pos.close();

  pack_trm.close();
  pack_doc.close();
  pack_pos.close();
  pack_tmap.close();

  remove((m1+".0").c_str());
  remove((m2+".0").c_str());
  remove((m3+".0").c_str());
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
  ofstream merge_trm, merge_doc, merge_pos;
  ifstream tmp_files[numtmps*3];
  map<pair<string, int>, TermAttr> termheap;
  string prefix = path+"/"+POSTINGS_FILE, m1, m2, m3;
  m1 = prefix+".trm."+itoa(destmp);
  m2 = prefix+".doc."+itoa(destmp);
  m3 = prefix+".pos."+itoa(destmp);

  merge_trm.open((m1+".tmp").c_str());
  merge_doc.open((m2+".tmp").c_str(), ios::binary);
  merge_pos.open((m3+".tmp").c_str(), ios::binary);

  for (int i = 0, j = headtmp; i < numtmps; ++i, ++j) {
    string s1, s2, s3;
    s1 = prefix+".trm."+itoa(j);
    s2 = prefix+".doc."+itoa(j);
    s3 = prefix+".pos."+itoa(j);
    tmp_files[i*3  ].open(s1.c_str(), ios::binary);
    tmp_files[i*3+1].open(s2.c_str(), ios::binary);
    tmp_files[i*3+2].open(s3.c_str(), ios::binary);
    if (!tmp_files[i*3] || !tmp_files[i*3+1] || !tmp_files[i*3+2]) {
      error("Writer::fail opening %s %s %s", s1.c_str(), s2.c_str(), s3.c_str());
    }
  }
  for (int i = 0; i < numtmps; ++i) {
    ifstream& ftrm = tmp_files[i*3];
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
      ifstream& ftrm = tmp_files[i*3];
      ifstream& fdoc = tmp_files[i*3+1];

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
      ifstream& fdoc = tmp_files[i*3+1];
      ifstream& fpos = tmp_files[i*3+2];

      int ndoc, npos;
      fread(fdoc, &ndoc, sizeof(ndoc));
      fread(fdoc, &didbuf[didupto], sizeof(didbuf[0])*ndoc);
      didupto += ndoc;

      while(ndoc--) {
        fread(fpos, &npos, sizeof(npos));
        fread(fpos, posbuf, sizeof(posbuf[0])*npos);
        assert(npos > 0 && npos < PST_BUF);
        fwrite(merge_pos, &npos, sizeof(npos));
        fwrite(merge_pos, posbuf, sizeof(posbuf[0])*npos);
        posupto += npos;
      }
    }
    assert(didupto == num_docs && didupto < PST_BUF);

    fwrite(merge_doc, &num_docs, sizeof(num_docs));
    fwrite(merge_doc, didbuf, sizeof(didbuf[0])*num_docs);

    TermAttr attr;
    attr.str = head.first;
    attr.df = didupto;
    attr.cf = posupto;
    attr.flush(merge_trm);

    num_term++;
  }
  for (int i = 0, j = headtmp; i < numtmps; ++i, ++j) {
    tmp_files[i*3 ].close();
    tmp_files[i*3+1].close();
    tmp_files[i*3+2].close();
    remove((prefix+".trm."+itoa(j)).c_str());
    remove((prefix+".doc."+itoa(j)).c_str());
    remove((prefix+".pos."+itoa(j)).c_str());
  }
  merge_trm.close();
  merge_doc.close();
  merge_pos.close();
  remove(m1.c_str());
  remove(m2.c_str());
  remove(m3.c_str());
  rename((m1+".tmp").c_str(), m1.c_str());
  rename((m2+".tmp").c_str(), m2.c_str());
  rename((m3+".tmp").c_str(), m3.c_str());
}


void IndexWriter::writePST() {
  ofstream fdmap;
  fdmap.open((path+"/"+DOC_MAP_FILE).c_str(), ios::binary);
  unsigned numtmps = 0;  // num of intermediate files
  unsigned numpsts = 0;  // num of position psts(to guess memory overhead)
  unsigned numfiles = 0;

  set<string> wordset;                            // {word}
  map<string, map<int, vector<int> > > postings;  // {term => {did => [pos]}}

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
      if (!t.length()) {
        continue;
      }
      postings[t][did].push_back(j);
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

void IndexWriter::writeGRAMS() {
  map<string, vector<int> > grams;  // k-gram index
  ifstream fwmap;
  string word;
  int wid, cnt = 0;

  PermuTree tree;
  tree.init(path+"/"+PERMUTERM_FILE);
  fwmap.open((path+"/"+WORD_MAP_FILE).c_str());

  while (fwmap >> wid >> word) {

    if ((++cnt) % 10000 == 0) {
      cout << "(" << cnt << ")" << endl;
    }

    string& w = word;
    int sz = w.length();
    // k-gram
    for (int k = MIN_N_GRAM; k <= MAX_N_GRAM; ++k) {
      for (int n = 0; n < sz - 1; ++n) {
        string g = w.substr(n, k);
        map<string, vector<int> >::iterator it;
        it = grams.find(g);
        vector<int>& v = grams[g];
        if (v.empty() || *(v.rbegin()) != wid) {
          v.push_back(wid);
        }
      }
    }
    if (sz + 1 >= PERMU_BUF) {
      continue;
    }
    // permuterm, only store words without exact match
    w = w+"$"+w;
    for (int n = 0; n <= sz; ++n) {
      string term = w.substr(n, sz + 1);
      Permuterm pterm(term, wid);
      tree.insert(pterm);
    }
  }
  fwmap.close();

  map<string, vector<int> >::iterator it;
  ofstream fout;
  fout.open((path+"/"+GRAMS_FILE).c_str());
  for (it = grams.begin(); it != grams.end(); ++it) {
    fout << it->first << " " << it->second.size() << "\n";
    vector<int> &v = it->second;
    for (unsigned i = 0; i < v.size(); ++i) {
      fout << " " << v[i];
    }
    fout << "\n";
  }
  fout.close();
}
