#include <cmath>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include "search/suggester.h"
#include "search/searcher.h"
#include "search/scorer.h"
#include "query/parser.h"

using namespace std;

int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "usage: "<< argv[0] << " <index path>" <<endl;
    return 1;
  }
  string indx_path = argv[1];

  FileLoader fl(indx_path);
  IndexReader ir(indx_path);
  IndexSearcher is(ir);
  Scorer sc(ir);
  Parser p;
  string query;
  int qid;

  while (cin >> qid) {
    getline(cin, query);
    cerr << qid << " " << query;
    Query* q = p.parse(query);
    cerr << " [" << q->tostring() << "]" << endl;
  
    is.search(q);
    sc.init(q);

    //vector<pair<int, float> > &result = sc.score(Model_OKAPI);
    vector<pair<int, float> > &result = sc.score(Model_LMDIRI);
    for (unsigned i = 0; i < result.size() && i < 10000; i++) {
      int did = result[i].first;
      float score = result[i].second;
      cout << qid << " Q0 " << ir.didmap[did].name << " 0 " << score;
      cout << " SleepSort" << endl;
    }

    delete q;
  }
  return 0;
}
