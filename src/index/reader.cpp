#include "index/reader.h"
IndexReader::IndexReader(string path) {
  this->path = path;
  this->buf = new unsigned[1048576];
  this->fpbuf = new unsigned long long[1048576];
  read();
}
IndexReader::~IndexReader() {
  delete []buf;
  delete []fpbuf;
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
  /*
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
  fin.close();*/

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
    posfp[old].clear();
    postings[old].clear();
    posfp.erase(old);
    postings.erase(old);
    pst_pool.erase(old);
    pst_queue.pop();
  }

  string prefix = path+"/"+IndexWriter::POSTINGS_FILE;
  ifstream fdoc; 
  long long fpdoc;
  int ndoc;

  fdoc.open((prefix+".doc").c_str(), ios::binary);
  fpdoc = docfp[tid];
  fseekg(fdoc, fpdoc, ios::beg);
  fread(fdoc, &ndoc, sizeof(ndoc));

  fread(fdoc, buf, sizeof(buf[0])*ndoc);       // need bulk read
  fread(fdoc, fpbuf, sizeof(fpbuf[0])*ndoc);   // need bulk read

  for (int i = 0; i < ndoc ; i++) {
    int did = buf[i];
    posfp[tid][did] = fpbuf[i];
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
  unsigned buf[1048576];
  assert(pst_pool.find(tid) != pst_pool.end());

  string prefix = path+"/"+IndexWriter::POSTINGS_FILE;
  long long fppos = posfp[tid][did];
  ifstream fpos; 
  int npos, pos;
  
  fpos.open((prefix+".pos").c_str(), ios::binary);
  fseekg(fpos, fppos, ios::beg);
  fread(fpos, &npos, sizeof(npos));
  vector<int> &v = postings[tid][did];

  fread(fpos, buf, sizeof(buf[0])*npos);
  v.insert(v.begin(), buf, buf+npos);
  
  while (npos--) {
    fread(fpos, &pos, sizeof(pos));
    v.push_back(pos);
  }
  pst_pool[tid][did] = true;
  fpos.close();
}
