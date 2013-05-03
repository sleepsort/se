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
  rmdir(path.c_str());
  if (mkdir(path.c_str(), S_IRWXU) != 0) {
    error("Writer::fail create index directory: %s", path.c_str());
  }
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
    fwmap << *it << endl;
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
    int ndoc = it->second.size();
    int accum = 0;

    fwrite(fdoc, &ndoc, sizeof(ndoc));

    for (jt = it->second.begin(); jt != it->second.end(); ++jt) {
      int did = jt->first;
      int npos = jt->second.size();
      copy(jt->second.begin(), jt->second.begin()+npos, posbuf);

      fwrite(fdoc, &did, sizeof(did));
      fwrite(fpos, &npos, sizeof(npos));
      fwrite(fpos, posbuf, sizeof(posbuf[0]) * npos);
      accum += npos;
    }
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

void IndexWriter::mergeWMAPBlk(int numtmps) {
  string prefix = path+"/"+WORD_MAP_FILE;
  ifstream tmp_files[numtmps];
  ofstream merge_wmap;

  merge_wmap.open(prefix.c_str());
  for (int i = 0; i < numtmps; ++i) {
    tmp_files[i].open((prefix+"."+itoa(i)).c_str());
  }

  set<pair<string, int> > wordheap;
  for (int i = 0; i < numtmps; ++i) {
    ifstream& wmap = tmp_files[i];
    string word;
    if (wmap >> word) {
      wordheap.insert(make_pair(word, i));
    }
  }
  int num_word = 0;
  while (!wordheap.empty()) {
    set<pair<string, int> >::iterator it = wordheap.begin();
    pair<string, int> head = *it;

    merge_wmap << num_word << " " << head.first << endl;
    num_word++;
    while (!it->first.compare(head.first)) {
      unsigned i = it->second;
      ifstream& wmap = tmp_files[i];
      string word;
      if (wmap >> word) {
        wordheap.insert(make_pair(word, i));
      }
      wordheap.erase(it);
      if (wordheap.empty()) {
        break;
      }
      it = wordheap.begin();
    }
  }
  for (int i = 0; i < numtmps; ++i) {
    tmp_files[i].close();
    remove((prefix+"."+itoa(i)).c_str());
  }
  merge_wmap.close();
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
void IndexWriter::mergePSTBlk(int numtmps) {
  cout << "merging..." << endl;
  ofstream merge_trm, merge_doc, merge_pos, merge_tmap;
  ifstream tmp_files[numtmps*3];
  map<pair<string, int>, TermAttr> termheap;
  string prefix = path+"/"+POSTINGS_FILE;

  merge_trm.open((prefix+".trm").c_str());
  merge_doc.open((prefix+".doc").c_str(), ios::binary);
  merge_pos.open((prefix+".pos").c_str(), ios::binary);
  merge_tmap.open((path+"/"+TERM_MAP_FILE).c_str(), ios::binary);

  for (int i = 0; i < numtmps; ++i) {
    tmp_files[i*3].open((prefix+".trm."+itoa(i)).c_str(), ios::binary);
    tmp_files[i*3+1].open((prefix+".doc."+itoa(i)).c_str(), ios::binary);
    tmp_files[i*3+2].open((prefix+".pos."+itoa(i)).c_str(), ios::binary);
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

    //merge_tmap << num_term << " " << head.first << endl;
    merge_trm  << num_term << " " << ftellp(merge_doc) << endl;

    while (!it->first.first.compare(head.first)) {
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
      if (termheap.empty())
        break;
      it = termheap.begin();
    }

    int posupto = 0, didupto = 0, fpupto = 0, size;
    set<int>::iterator jt;
    for (jt = hit.begin(); jt != hit.end(); jt++) {
      unsigned i = *jt;
      ifstream& fdoc = tmp_files[i*3+1];
      ifstream& fpos = tmp_files[i*3+2];
      int ndoc, npos;

      fread(fdoc, &ndoc, sizeof(ndoc));
      assert(ndoc > 0);

      fread(fdoc, &didbuf[didupto], sizeof(didbuf[0])*ndoc);
      didupto += ndoc;

      while(ndoc--) {
        fpbuf[fpupto++] = ftellp(merge_pos);

        fread(fpos, &npos, sizeof(npos));
        fread(fpos, posbuf, sizeof(posbuf[0])*npos);

        assert(npos > 0 && npos < PST_BUF);
        posupto += npos;
  
        dgap(posbuf, npos);
        encode_vb(posbuf, npos, buf, size);
        assert(size < PST_BUF*2);
        fwrite(merge_pos, &size, sizeof(size));
        fwrite(merge_pos, buf, sizeof(buf[0])*size);
      }
    }
    assert(fpupto == num_docs && didupto == num_docs && didupto < PST_BUF);

    dgap(didbuf, num_docs);
    encode_vb(didbuf, num_docs, buf, size);
    assert(size < PST_BUF*10);
    fwrite(merge_doc, &size, sizeof(size));
    fwrite(merge_doc, buf, sizeof(buf[0])*size);

    dgap(fpbuf, num_docs);
    encode_vb(fpbuf, num_docs, buf, size);
    assert(size < PST_BUF*10);
    fwrite(merge_doc, &size, sizeof(size));
    fwrite(merge_doc, buf, sizeof(buf[0])*size);

    TermAttr attr;
    attr.str = head.first;
    attr.df = didupto;
    attr.cf = posupto;
    attr.flush(merge_tmap);

    num_term++;
  }
  for (int i = 0; i < numtmps; ++i) {
    tmp_files[i*3].close();
    tmp_files[i*3+1].close();
    tmp_files[i*3+2].close();
    remove((prefix+".trm."+itoa(i)).c_str());
    remove((prefix+".doc."+itoa(i)).c_str());
    remove((prefix+".pos."+itoa(i)).c_str());
  }
  merge_trm.close();
  merge_doc.close();
  merge_pos.close();
  merge_tmap.close();
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

      postings[t][did].push_back(j);
      numpsts++;
    }
    if (numpsts > 1e7) {
      tick(); 
      flushPSTBlk(postings, numtmps);
      flushWMAPBlk(wordset, numtmps);
      tock(); 
      numtmps++;
      numpsts = 0;
      postings.clear();
      wordset.clear();
    }
    numfiles++;
  }
  if (numpsts > 0) {
    tick(); 
    flushPSTBlk(postings, numtmps);
    flushWMAPBlk(wordset, numtmps);
    tock(); 
    numtmps++;
    numpsts = 0;
    postings.clear();
    wordset.clear();
  }
  tick();
  mergePSTBlk(numtmps);
  mergeWMAPBlk(numtmps);
  tock();
  fdmap.close();
}

void IndexWriter::writeGRAMS() {
  map<string, vector<int> > grams;  // k-gram index
  ifstream fwmap;
  string word;
  int wid;

  PermuTree tree;
  tree.init(path+"/"+PERMUTERM_FILE);
  fwmap.open((path+"/"+WORD_MAP_FILE).c_str());

  while (fwmap >> wid >> word) {
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
    fout << it->first << " " << it->second.size() << endl;
    vector<int> &v = it->second;
    for (unsigned i = 0; i < v.size(); ++i) {
      fout << " " << v[i];
    }
    fout << endl;
  }
  fout.close();
}
