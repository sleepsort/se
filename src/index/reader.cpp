#include "index/reader.h"
IndexReader::IndexReader(const string &path) {
  this->path = path;
  this->cbuf = new char[PST_BUF*10];
  this->ibuf = new unsigned[PST_BUF];
  this->fbuf = new unsigned[PST_BUF];
  read();
}
IndexReader::~IndexReader() {
  delete []cbuf;
  delete []ibuf;
  delete []fbuf;
}


void IndexReader::read() {
  ifstream fin;
  string token;
  int wid, tid, did;
  long long fp;
  map<int, map<int, vector<int> > >::iterator it;
  map<int, vector<int> >::iterator jt;
  map<string, vector<int> >::iterator kt;

  fin.open((path+"/"+IndexWriter::TERM_MAP_FILE).c_str(), ios::binary);
  if (!fin) {
    cerr << "WARNING::Reader::";
    cerr << (path+"/"+IndexWriter::TERM_MAP_FILE) << " not found" << endl;
  } else {
    TermAttr ta;
    tid = 0;
    while (ta.load(fin) >= 0) {
      termmap.insert(make_pair(ta.str, tid));
      tidmap.push_back(ta);
      tid++;
    }
    cerr << "# num_term = " << tid << endl;
    fin.close();
  }

  fin.open((path+"/"+IndexWriter::DOC_MAP_FILE).c_str());
  if (!fin) {
    cerr << "WARNING::Reader::";
    cerr << (path+"/"+IndexWriter::DOC_MAP_FILE) << " not found" << endl;
  } else {
    DocAttr da;
    did = 0;
    ttf = 0;
    while (da.load(fin) >= 0) {
      didmap.push_back(da);
      ttf+=da.len;
      did++;
    }
    cerr << "# num_docs = " << did << endl;
    cerr << "# tot_term = " << ttf << endl;
    fin.close();
  }

  fin.open((path+"/"+IndexWriter::POSTINGS_FILE+".trm").c_str());
  if (!fin) {
    cerr << "WARNING::Reader::";
    cerr << (path+"/"+IndexWriter::POSTINGS_FILE+".trm") << " not found" << endl;
  } else {
    while (fin >> tid) {
      fin >> fp;
      docfp.push_back(fp);
    }
    fin.close();
  }

  fin.open((path+"/"+IndexWriter::WORD_MAP_FILE).c_str());
  if (!fin) {
    cerr << "WARNING::Reader::";
    cerr << (path+"/"+IndexWriter::WORD_MAP_FILE) << " not found" << endl;
  } else {
    while (fin >> wid) {
      fin.ignore();
      getline(fin, token);
      wordmap.insert(make_pair(token, wid));
      widmap.insert(make_pair(wid, token));
    }
    fin.close();
  }


  fin.open((path+"/"+IndexWriter::GRAMS_FILE).c_str());
  if (!fin) {
    cerr << "WARNING::Reader::";
    cerr << (path+"/"+IndexWriter::GRAMS_FILE) << " not found" << endl;
  } else {
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

  //permutree.init(path+"/"+IndexWriter::PERMUTERM_FILE);
}

void IndexReader::filldoc(int tid) {
  if (pst_pool.find(tid) != pst_pool.end()) {
    return;
  }
  if (pst_queue.size() > 6) {
    int old = pst_queue.front();

    postings.erase(old);
    pst_pool.erase(old);

    pst_queue.pop();
    malloc_trim(0);  // otherwise not really cleared
  }

  string prefix = path+"/"+IndexWriter::POSTINGS_FILE;
  ifstream fdoc; 
  long long fpdoc;
  int ndoc = 0, npos = 0, size = 0;

  fdoc.open((prefix+".doc").c_str(), ios::binary);
  if (!fdoc) {
    cerr << "WARNING::Reader::";
    cerr << (prefix+".doc") << " not found" << endl;
    return;
  }
  fpdoc = docfp[tid];
  fseekg(fdoc, fpdoc, ios::beg);

  fread(fdoc, &size, sizeof(size));
  fread(fdoc, cbuf, sizeof(cbuf[0])*size);
  decode_vb(cbuf, size, ibuf, ndoc);
  ungap(ibuf, ndoc);

  fread(fdoc, &size, sizeof(size));
  fread(fdoc, cbuf, sizeof(cbuf[0])*size);
  decode_vb(cbuf, size, fbuf, ndoc);

  for (int i = 0; i < ndoc ; ++i) {
    int did = ibuf[i];
    int frq = fbuf[i];
    postings[tid][did] = frq;
    npos += frq;
  }
  assert(tidmap[tid].cf == npos || !(cout << tidmap[tid].cf << " " << npos << endl));
  if (pst_pool.find(tid) == pst_pool.end()) {
    pst_pool[tid] = 1;
  }
  pst_queue.push(tid);

  fdoc.close();
}
void IndexReader::fillpos(int tid, int did) {
  /*
  if (pst_pool.find(tid) != pst_pool.end() && pst_pool[tid][did]) {
    return;
  }
  assert(pst_pool.find(tid) != pst_pool.end());

  string prefix = path+"/"+IndexWriter::POSTINGS_FILE;
  long long fppos = posfp[tid][did];
  ifstream fpos; 
  int size, npos;
  
  fpos.open((prefix+".pos").c_str(), ios::binary);
  if (!fpos) {
    cerr << "WARNING::Reader::";
    cerr << (prefix+".doc") << " not found" << endl;
    return;
  }
  fseekg(fpos, fppos, ios::beg);
  vector<int> &v = postings[tid][did];

  fread(fpos, &size, sizeof(size));
  fread(fpos, cbuf, sizeof(cbuf[0])*size);   // need bulk read
  decode_vb(cbuf, size, ibuf, npos);
  ungap(ibuf, npos);

  v.insert(v.begin(), ibuf, ibuf+npos);
  
  pst_pool[tid][did] = true;
  fpos.close();
  */
}
