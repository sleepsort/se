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
            map<string, int>::iterator it;
            string &t = words[j];
            int tid;

            transform(t.begin(), t.end(), t.begin(), ::tolower);
            if ((it = termmap.find(t)) != termmap.end()) {
                tid = it->second;
            } else {
                termmap.insert(make_pair(t,numterms));
                tid = numterms++;
            }

            DocEnum* de = postings.docs(tid);
            PosEnum* pe;
            if (de == postings.end()) {
                de = new DocEnum(tid); 
                pe = new PosEnum(did);
                pe->add(j);
                de->add(pe);
                postings.add(tid, de);
            } else {
                pe = de->at(de->size()-1);
                pe->add(j);
            }
        }
        numdocs++;
    }
}
void IndexWriter::flush() {
    rmdir(path.c_str());
    int status = mkdir(path.c_str(), S_IRWXU);
    if (status != 0) {
        cerr << "Writer::fail create index directory: " << path << endl;
        return;
    }
    map<string, int>::iterator it;
    DocEnum *de;
    PosEnum *pe;
    int pos;
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

    postings.reset();
    while ((de = postings.next()) != postings.end()) {
        fout << de->id() << " " << de->size() << endl;
        while ((pe = de->next()) != de->end()) {
            fout << "  " << pe->id() << " " << pe->size() << endl << "   ";
            while ((pos = pe->next()) != pe->end()) {
                fout << pos << " ";
            }
            fout << endl;
        }
    }
    fout.close();
}
