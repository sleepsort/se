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

  rmdir(indx_path.c_str());
  if (mkdir(indx_path.c_str(), S_IRWXU) != 0) {
    error("Indexer::fail create index directory: %s", indx_path.c_str());
  }

  FileLoader fl(indx_path, data_path, data_type);
  IndexWriter iw(fl, indx_path);

  tick();
  iw.write();
  tock();

  return 0;
}
