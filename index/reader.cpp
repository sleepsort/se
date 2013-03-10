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
        DocEnum* de = new DocEnum(tid);
        postings.add(tid, de);

        for (int i=0; i<n; ++i) {
            fin >> did >> m;
            PosEnum* pe = new PosEnum(did);
            de->add(pe);
            for (int j = 0; j<m; ++j) {
                fin >> pos;
                pe->add(pos);
            }
        }
    }
    fin.close();
}
