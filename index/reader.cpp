#include "reader.h"
IndexReader::IndexReader(string path) {
    this->path = path;
}
IndexReader::~IndexReader() {
}
void IndexReader::read() {
    ifstream fin;

    string token;
    int tid, did, pos, n, m;
    map<int, map<int, vector<int> > >::iterator it;
    map<int, vector<int> >::iterator jt;

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
        postings.insert(make_pair(tid, map<int, vector<int> >()));
        it = postings.find(tid);

        for (int i=0; i<n; ++i) {
            fin >> did >> m;
            it->second.insert(make_pair(did, vector<int>()));
            jt = it->second.find(did);
            for (int j = 0; j<m; ++j) {
                fin >> pos;
                jt->second.push_back(pos);
            }
        }
    }
    fin.close();
}
