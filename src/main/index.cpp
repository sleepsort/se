#include <sys/time.h>
#include <iostream>
#include "index/writer.h"
#include "util/util.h"
using namespace std;

int main(int argc, char **argv) {
    if (argc != 3) {
        cout << "usage: " << argv[0] << " <data path> <index path>" << endl;
        return 1;
    }
    string data_path = argv[1];
    string indx_path = argv[2];

    vector<string> files;
    set<string> ex;

    ex.insert("README");
    ex.insert("glossary");

    timeval start, end;
    gettimeofday(&start, NULL);

    collect(data_path, files, ex);

    IndexWriter iw(indx_path);
    iw.write(files);
    iw.flush();

    gettimeofday(&end, NULL);
    double t = 0;
    t += end.tv_sec - start.tv_sec;
    t += (double)(end.tv_usec-start.tv_usec)/1000000.0;
    printf("building index for '%s' takes: %.3fs\n",data_path.c_str(),t);

    return 0;
}
