#include <cmath>
#include <iostream>
#include <iomanip>
#include "search/summarizer.h"
#include "search/suggester.h"
#include "search/searcher.h"
#include "search/scorer.h"
#include "query/parser.h"

using namespace std;

int padding;

void report(vector<pair<int, double> > result, Query *q, IndexReader &ir, Summarizer &sr) {
  cout << "numhit = " << result.size() << endl;
  for (unsigned i = 0; i < result.size() && i < 5; i++) {
    int did = result[i].first;
    cout << "[" << setw(padding) << setfill(' ') << did << "] ";
    cout << fixed << setprecision(6) << result[i].second << " ";
    cout << ir.didmap[did].name << " ";
    cout << "(" << ir.didmap[did].len << ")" << endl;
    cout << "  ..." << sr.summary(q, did) << "..." << endl;
  }
  if (result.size() > 5) {
    cout << "... (result trucated)" << endl;
  }
  cout << endl;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "usage: "<< argv[0] << " <index path>" <<endl;
    return 1;
  }
  string indx_path = argv[1];

  FileLoader fl(indx_path);
  IndexReader ir(indx_path);
  Summarizer sr(ir, fl);
  IndexSearcher is(ir);
  Scorer sc(ir);
  Parser p;
  string query;

  padding = log10(ir.didmap.size())+1;

  while (getline(cin, query)) {
    Query* q = p.parse(query);
    is.search(q);
    sc.init(q);
    cout << "query = " << q->tostring() << endl << endl;

    vector<pair<int, double> > &result1 = sc.score(Model_VSM);
    cout << "VSM:     ";
    report(result1, q, ir, sr);

    vector<pair<int, double> > &result2 = sc.score(Model_OKAPI);
    cout << "BM25:    ";
    report(result2, q, ir, sr);

    vector<pair<int, double> > &result3 = sc.score(Model_LMJM);
    cout << "LM+JM:   ";
    report(result3, q, ir, sr);

    vector<pair<int, double> > &result4 = sc.score(Model_LMDIRI);
    cout << "LM+Diri: ";
    report(result4, q, ir, sr);

    cout << endl;

    delete q;
  }
  return 0;
}
