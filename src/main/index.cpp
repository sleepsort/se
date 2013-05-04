#include <sys/time.h>
#include <iostream>
#include "index/loader.h"
#include "index/writer.h"
#include "util/file.h"
#include "util/debug.h"
using namespace std;

int main(int argc, char **argv) {
  if (argc != 4) {
    cout << "usage: " << argv[0] << " <data path> <data type> <index path>" << endl;
    return 1;
  }
  string data_path = argv[1];
  string data_type = argv[2];
  string indx_path = argv[3];

  Corpus type; 
  if (!data_type.compare("shakes")) {
    type = CORPUS_SHAKES;
  } else if (!data_type.compare("rcv1")) {
    type = CORPUS_RCV1;
  } else if (!data_type.compare("gov2")) {
    type = CORPUS_GOV2;
  } else {
    type = CORPUS_RAW;
  }

  FileLoader fl(data_path, type);
  IndexWriter iw(fl, indx_path);

  tick();
  iw.write();
  tock();

  return 0;
}
