#include <iostream>
#include "util/string.h"
using namespace std;

int main(int argc, char **argv) {
  string line;
  int qid;
  while (getline(cin, line)) {
    size_t pos;
    if ((pos = line.find("Number:")) != string::npos) {
      qid = atoi(line.substr(pos+7).c_str());
      cout << qid << " ";
      continue;
    }
    if ((pos = line.find("<title>")) != string::npos) {
      line = line.substr(pos+7);
      trim(line);
      if (!line.length()) {
        getline(cin, line);
        trim(line);
      }
      cout << line << endl;
    }
  }
  return 0;
}
