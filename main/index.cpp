#include <cstring>
#include <iostream>
#include <algorithm>
#include "index/writer.h"
#include "util/util.h"
using namespace std; 

int main(int argc, char **argv) {
    if (argc != 3) {
        cout << "usage: "<<argv[0]<<" <data path> <index path>" <<endl;
        return 1;
    }
    string data_path = argv[1];
    string index_path = argv[2];

    vector<string> files;
    set<string> ex;

    ex.insert("README");
    ex.insert("glossary");

    collect(data_path, files, ex);

    IndexWriter iw(index_path);
    iw.write(files);
    iw.flush();

    return 0;
}
