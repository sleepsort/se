#include "writer.h"

const string IndexWriter::WORD_MAP_FILE = "vmap.dat";
const string IndexWriter::TERM_MAP_FILE = "tmap.dat";
const string IndexWriter::DOC_MAP_FILE = "dmap.dat";
const string IndexWriter::GRAMS_FILE = "grm.dat";
const string IndexWriter::POSTINGS_FILE = "pst.dat";

IndexWriter::IndexWriter(string path) {
    this->path = path;
    numdocs = 0;
    numterms = 0;
    numwords = 0;
}
IndexWriter::~IndexWriter() {
}
void IndexWriter::write(vector<string>& files) {
    unsigned numfiles = files.size();

    for (unsigned i=0; i<numfiles; ++i) {
        vector<string> words;
        int did = numdocs;
        unsigned n;

        docmap.insert(make_pair(files[i], did));
        didmap.insert(make_pair(did, files[i]));
        tokenize(files[i], words);
        n = words.size();

        for (unsigned j=0; j<n; ++j) {
            map<string, int>::iterator it;
            map<int, map<int, vector<int> > >::iterator jt;
            map<int, vector<int> >::iterator kt;
            string t = words[j];
            int tid;

            lowercase(t);
            if ((it = wordmap.find(t)) == wordmap.end()) {
                wordmap.insert(make_pair(t, numwords));
                vidmap.insert(make_pair(numwords, t));
                numwords++;
            }
            
            porterstem(t);
            if ((it = termmap.find(t)) != termmap.end()) {
                tid = it->second;
            } else {
                termmap.insert(make_pair(t,numterms));
                tidmap.insert(make_pair(numterms,t));
                tid = numterms++;
            }
    
            jt = postings.find(tid);
            if (jt == postings.end()) {
                postings.insert(make_pair(tid, map<int, vector<int> >()));
                jt = postings.find(tid);
            }
            kt = jt->second.find(did);
            if (kt == jt->second.end()) {
                jt->second.insert(make_pair(did, vector<int>()));
                kt = jt->second.find(did);
            }
            kt->second.push_back(j);
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
    map<int, string>::iterator it;
    map<int, map<int, vector<int> > >::iterator jt;
    map<int, vector<int> >::iterator kt;

    ofstream fout;

    fout.open((path+"/"+WORD_MAP_FILE).c_str());
    for (it = vidmap.begin(); it!=vidmap.end(); ++it) {
        fout<<it->first<<" "<<it->second<<endl;
    }
    fout.close();

    fout.open((path+"/"+TERM_MAP_FILE).c_str());
    for (it = tidmap.begin(); it!=tidmap.end(); ++it) {
        fout<<it->first<<" "<<it->second<<endl;
    }
    fout.close();

    fout.open((path+"/"+DOC_MAP_FILE).c_str());
    for (it = didmap.begin(); it!=didmap.end(); ++it) {
        fout<<it->first<<" "<<it->second<<endl;
    }
    fout.close();

    fout.open((path+"/"+POSTINGS_FILE).c_str());

    for (jt = postings.begin(); jt != postings.end(); jt++) {
        fout << jt->first << " " << jt->second.size() << endl;
        for (kt = jt->second.begin(); kt != jt->second.end(); kt++) {
            fout << " " << kt->first << " " << kt->second.size() << endl << "  ";
            for (unsigned i = 0; i < kt->second.size(); i++) {
                fout << kt->second[i] << " ";
            }
            fout << endl;
        }
    }
    fout.close();
}
