#include "index/reader.h"
IndexReader::IndexReader(string path) {
  this->path = path;
  this->cbuf = new char[PST_BUF*10];
  this->ibuf = new unsigned[PST_BUF];
  this->lbuf = new unsigned long long[PST_BUF];
  read();
}
IndexReader::~IndexReader() {
  delete []cbuf;
  delete []ibuf;
  delete []lbuf;
}


void IndexReader::read() {
  ifstream fin;
  string token;
  int wid, tid, did;
  long long fp;
  map<int, map<int, vector<int> > >::iterator it;
  map<int, vector<int> >::iterator jt;
  map<string, vector<int> >::iterator kt;

  fin.open((path+"/"+IndexWriter::WORD_MAP_FILE).c_str());
  while (fin >> wid) {
    fin.ignore();
    getline(fin, token);
    wordmap.insert(make_pair(token, wid));
    widmap.insert(make_pair(wid, token));
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

  fin.open((path+"/"+IndexWriter::POSTINGS_FILE+".trm").c_str());
  while (fin >> tid) {
    fin >> fp;
    docfp.insert(make_pair(tid,fp));
  }
  fin.close();

  fin.open((path+"/"+IndexWriter::GRAMS_FILE).c_str());
  while (getline(fin, token, ' ')) {
    int n;
    fin >> n;
    grams.insert(make_pair(token, vector<int>()));
    kt = grams.find(token);
    for (int i = 0; i < n; ++i) {
      fin >> wid;
      kt->second.push_back(wid);
    }
    fin.ignore();
  }
  fin.close();
}

void IndexReader::filldoc(int tid) {
  if (pst_pool.find(tid) != pst_pool.end()) {
    return;
  }
  if (pst_queue.size() > 8) {
    int old = pst_queue.front();

    posfp.erase(old);
    postings.erase(old);
    pst_pool.erase(old);

    pst_queue.pop();
    malloc_trim(0);  // otherwise not really cleared
  }

  string prefix = path+"/"+IndexWriter::POSTINGS_FILE;
  ifstream fdoc; 
  long long fpdoc;
  int ndoc, size;

  fdoc.open((prefix+".doc").c_str(), ios::binary);
  fpdoc = docfp[tid];
  fseekg(fdoc, fpdoc, ios::beg);

  fread(fdoc, &size, sizeof(size));
  fread(fdoc, cbuf, sizeof(cbuf[0])*size);       // need bulk read
  decode_vb(cbuf, size, ibuf, ndoc);
  ungap(ibuf, ndoc);

  fread(fdoc, &size, sizeof(size));
  fread(fdoc, cbuf, sizeof(cbuf[0])*size);   // need bulk read
  decode_vb(cbuf, size, lbuf, ndoc);
  ungap(lbuf, ndoc);

  for (int i = 0; i < ndoc ; ++i) {
    int did = ibuf[i];
    posfp[tid][did] = lbuf[i];
    postings[tid].insert(make_pair(did, vector<int>()));
    if (pst_pool.find(tid) == pst_pool.end()) {
      pst_pool[tid][did] = false;
    }
  }
  pst_queue.push(tid);

  fdoc.close();
}
void IndexReader::fillpos(int tid, int did) {
  if (pst_pool.find(tid) != pst_pool.end() && pst_pool[tid][did]) {
    return;
  }
  assert(pst_pool.find(tid) != pst_pool.end());

  string prefix = path+"/"+IndexWriter::POSTINGS_FILE;
  long long fppos = posfp[tid][did];
  ifstream fpos; 
  int size, npos;
  
  fpos.open((prefix+".pos").c_str(), ios::binary);
  fseekg(fpos, fppos, ios::beg);
  vector<int> &v = postings[tid][did];

  fread(fpos, &size, sizeof(size));
  fread(fpos, cbuf, sizeof(cbuf[0])*size);   // need bulk read
  decode_vb(cbuf, size, ibuf, npos);
  ungap(ibuf, npos);

  v.insert(v.begin(), ibuf, ibuf+npos);
  
  pst_pool[tid][did] = true;
  fpos.close();
}
