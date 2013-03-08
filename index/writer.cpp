#include "writer.h"

const string IndexWriter::TERM_MAP_FILE = "tmap.dat";
const string IndexWriter::DOC_MAP_FILE = "dmap.dat";
const string IndexWriter::POSTINGS_FILE = "pst.dat";

IndexWriter::IndexWriter(string path) {
    this->path = path;
    numdocs = 0;
    numterms = 0;
}
IndexWriter::~IndexWriter() {
}
void IndexWriter::write(vector<string>& files) {
    unsigned numfiles = files.size();

    for (unsigned i=0; i<numfiles; ++i) {
        vector<string> words(1024);
        int did = numdocs;
        unsigned n;

        docmap.insert(make_pair(files[i], did));
        tokenize(files[i], words);
        n = words.size();

        for (unsigned j=0; j<n; ++j) {
            string &t = words[j];
            map<string, int>::iterator it;
            int tid;

            transform(t.begin(), t.end(), t.begin(), ::tolower);
            if ((it = termmap.find(t)) != termmap.end()) {
                tid = it->second;
            } else {
                termmap.insert(make_pair(t,numterms));
                tid = numterms++;
            }
            vector<int> &posting = postingslist[tid];
            if (!posting.size() || *(posting.rbegin()) != did) {
                posting.push_back(did);
            }
        }
        numdocs++;
    }
}
void IndexWriter::flush() {
    rmdir(path.c_str());
    int status = mkdir(path.c_str(), S_IRWXU);
    if (status != 0) {
        cerr << "fail create index directory: " << path << endl;
        return;
    }
    map<string, int>::iterator it;
    map<int, vector<int> >::iterator jt;
    ofstream fout;

    fout.open((path+"/"+TERM_MAP_FILE).c_str());
    for (it = termmap.begin(); it!=termmap.end(); ++it) {
        fout<<it->second<<" "<<it->first<<endl;
    }
    fout.close();

    fout.open((path+"/"+DOC_MAP_FILE).c_str());
    for (it = docmap.begin(); it!=docmap.end(); ++it) {
        fout<<it->second<<" "<<it->first<<endl;
    }
    fout.close();

    fout.open((path+"/"+POSTINGS_FILE).c_str());
    for (jt = postingslist.begin(); jt!=postingslist.end(); ++jt) {
        vector<int> &v = jt->second;
        unsigned sz = v.size();
        fout<<jt->first<<" "<<sz;
        for (unsigned i=0; i<sz; i++) {
            fout<<" "<<v[i];
        }
        fout<<endl;
    }
    fout.close();
}
