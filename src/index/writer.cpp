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
}
IndexWriter::~IndexWriter() {
}


void IndexWriter::write(const vector<string>& files) {
  rmdir(path.c_str());
  if (mkdir(path.c_str(), S_IRWXU) != 0) {
    cerr << "Writer::fail create index directory: " << path << endl;
    return;
  }
  writeDMAP(files);
  writePST(files);
  writeGRAMS();
}

void IndexWriter::writeDMAP(const vector<string>& files) {
  unsigned numfiles = files.size();

  ofstream fdmap;
  fdmap.open((path+"/"+DOC_MAP_FILE).c_str());
  for (unsigned i = 0; i < numfiles; ++i) {
    fdmap << i << " " << files[i] << endl;
  }
  fdmap.close();
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

  ftrm.open((prefix+".trm."+itoa(turn)).c_str());
  fdoc.open((prefix+".doc."+itoa(turn)).c_str(), ios::binary);
  fpos.open((prefix+".pos."+itoa(turn)).c_str(), ios::binary);

  map<string, map<int, vector<int> > >::iterator it;
  map<int, vector<int> >::iterator jt;

  for (it = pst.begin(); it != pst.end(); ++it) {
    string term = it->first;
    int ndoc = it->second.size();

    ftrm << term << endl;
    fwrite(fdoc, &ndoc, sizeof(ndoc));

    for (jt = it->second.begin(); jt != it->second.end(); ++jt) {
      int did = jt->first;
      int npos = jt->second.size();

      fwrite(fdoc, &did, sizeof(did));
      fwrite(fpos, &npos, sizeof(npos));

      for (unsigned i = 0; i < jt->second.size(); ++i) {
        int pos = jt->second[i];
        fwrite(fpos, &pos, sizeof(pos));
      }
    }
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
  for (int i = 0; i < numtmps; i++) {
    tmp_files[i].open((prefix+"."+itoa(i)).c_str());
  }

  set<pair<string, int> > wordheap;
  for (int i = 0; i < numtmps; i++) {
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
  for (int i = 0; i < numtmps; i++) {
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
  ofstream merge_trm, merge_doc, merge_pos, merge_tmap;
  ifstream tmp_files[numtmps*3];
  set<pair<string, int> > termheap;
  string prefix = path+"/"+POSTINGS_FILE;

  merge_trm.open((prefix+".trm").c_str());
  merge_doc.open((prefix+".doc").c_str(), ios::binary);
  merge_pos.open((prefix+".pos").c_str(), ios::binary);
  merge_tmap.open((path+"/"+TERM_MAP_FILE).c_str());

  for (int i = 0; i < numtmps; i++) {
    tmp_files[i*3].open((prefix+".trm."+itoa(i)).c_str());
    tmp_files[i*3+1].open((prefix+".doc."+itoa(i)).c_str(), ios::binary);
    tmp_files[i*3+2].open((prefix+".pos."+itoa(i)).c_str(), ios::binary);
  }
  for (int i = 0; i < numtmps; i++) {
    ifstream& ftrm = tmp_files[i*3];
    string term;
    if (ftrm >> term) {
      termheap.insert(make_pair(term, i));
    }
  }
  int num_term = 0;
  while (!termheap.empty()) {
    set<pair<string, int> >::iterator it = termheap.begin();
    pair<string, int> head = *it;
    set<int> hit;
    int num_docs = 0;

    merge_tmap << num_term << " " << head.first << endl;
    merge_trm  << num_term << " " << ftellp(merge_doc) << endl;
    num_term++;

    while (!it->first.compare(head.first)) {
      unsigned ndoc, i = it->second;
      ifstream& ftrm = tmp_files[i*3];
      ifstream& fdoc = tmp_files[i*3+1];
      string term;

      hit.insert(i);
      if (ftrm >> term) {
        termheap.insert(make_pair(term, i));
      }
      fpeek(fdoc, &ndoc, sizeof(ndoc));
      num_docs += ndoc;
      termheap.erase(it);
      if (termheap.empty())
        break;
      it = termheap.begin();
    }
    fwrite(merge_doc, &num_docs, sizeof(num_docs));
    set<int>::iterator jt;
    for (jt = hit.begin(); jt != hit.end(); jt++) {
      unsigned i = *jt;
      ifstream& fdoc = tmp_files[i*3+1];
      ifstream& fpos = tmp_files[i*3+2];
      int ndoc, npos, did, pos;
      long long fppos;
      fread(fdoc, &ndoc, sizeof(ndoc));
      assert(ndoc > 0);
      while(ndoc--) {
        fppos = ftellp(merge_pos);
        fread(fdoc, &did, sizeof(did));
        fread(fpos, &npos, sizeof(npos));
        assert(did >= 0 && npos > 0);
        fwrite(merge_doc, &did, sizeof(did));
        fwrite(merge_doc, &fppos, sizeof(fppos));
        fwrite(merge_pos, &npos, sizeof(npos));
        while(npos--) {
          fread(fpos, &pos, sizeof(pos));
          fwrite(merge_pos, &pos, sizeof(pos));
        }
      }
    }
  }
  for (int i = 0; i < numtmps; i++) {
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


void IndexWriter::writePST(const vector<string>& files) {
  unsigned numfiles = files.size();
  unsigned numtmps = 0;  // num of intermediate files
  unsigned numpsts = 0;  // num of position psts(to guess memory overhead)

  set<string> wordset;                            // {word}
  map<string, map<int, vector<int> > > postings;  // {term => {did => [pos]}}

  for (unsigned i = 0; i < numfiles; ++i) {
    if ((i+1) % 10000 == 0) {
      cout << "(" << i+1 << "/" << numfiles << ")" << endl;
    }
    vector<string> words;
    int did = i;
    unsigned n;
    if (extension(files[i]) == "xml") {
      xmltokenize(files[i], words);
    } else {
      rawtokenize(files[i], words);
    }
    n = words.size();

    for (unsigned j = 0; j < n; ++j) {
      string t = words[j];

      lowercase(t);
      wordset.insert(t);
      porterstem(t);

      map<string, map<int, vector<int> > >::iterator it = postings.find(t);
      if (it == postings.end()) {
        postings.insert(make_pair(t, map<int, vector<int> >()));
        it = postings.find(t);
      }

      map<int, vector<int> >::iterator jt = it->second.find(did);
      if (jt == it->second.end()) {
        it->second.insert(make_pair(did, vector<int>()));
        jt = it->second.find(did);
      }
      jt->second.push_back(j);
      numpsts++;
    }
    if (numpsts > 1e7) {
      flushPSTBlk(postings, numtmps);
      flushWMAPBlk(wordset, numtmps);
      numtmps++;
      numpsts = 0;
      postings.clear();
      wordset.clear();
    }
  }
  if (numpsts > 0) {
    flushPSTBlk(postings, numtmps);
    flushWMAPBlk(wordset, numtmps);
    numtmps++;
    numpsts = 0;
    postings.clear();
    wordset.clear();
  }
  mergePSTBlk(numtmps);
  mergeWMAPBlk(numtmps);
}

void IndexWriter::writeGRAMS() {
  map<string, vector<int> > grams;  // k-gram index
  map<string, vector<int> > permutermlist;  // un-structured permuterm

  ifstream fwmap((path+"/"+WORD_MAP_FILE).c_str());
  int wid;
  string word;

  while (fwmap >> wid >> word) {
    string& w = word;
    int sz = w.length();
    // k-gram
    for (int k = MIN_N_GRAM; k <= MAX_N_GRAM; ++k) {
      for (int n = 0; n < sz - 1; ++n) {
        string g = w.substr(n, k);
        map<string, vector<int> >::iterator it;
        it = grams.find(g);
        if (it == grams.end()) {
          grams.insert(make_pair(g, vector<int>()));
          it = grams.find(g);
        }
        vector<int>& v = it->second;
        if (v.empty() || *(v.rbegin()) != wid) {
          v.push_back(wid);
        }
      }
    }
    /*
    // permuterm, only store words without exact match
    w = w+"$"+w;
    for (int n = 0; n < sz; ++n) {
      string term = w.substr(n, sz + 1);
      permutermlist[term].push_back(wid);
    }*/
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
