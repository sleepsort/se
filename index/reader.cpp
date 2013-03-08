#include "reader.h"
IndexReader::IndexReader(string path) {
    this->path = path;
}
IndexReader::~IndexReader() {
}
void IndexReader::read() {
    ifstream fin;

    string token;
    int tid, did, n;

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
        fin >> n;
        vector<int> &v = postingslist[tid];
        for (int i=0; i<n; ++i) {
            fin >> did;
            v.push_back(did);
        }
    }
    fin.close();
}
