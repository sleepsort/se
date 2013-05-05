#include <cmath>
#include <iostream>
#include <iomanip>
#include "search/summarizer.h"
#include "search/suggester.h"
#include "search/searcher.h"
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
  Summarizer sr(ir, fl);
  IndexSearcher is(ir);
  Suggester ss(ir);
  Parser p;
  string query;
  int padding = log10(ir.didmap.size())+1;

  while (getline(cin, query)) {
    Query* q = p.parse(query);
    is.search(q);
    //is.report(q);
    cout << "query = " << q->tostring() << endl;
    cout << "numhit = " << q->docs().size() << endl;
    for (unsigned i = 0; i < q->docs().size() && i < 5; ++i) {
      int did = q->docs()[i];
      cout << "[" << setw(padding) << setfill(' ') << did << "] ";
      cout << ir.didmap[did].name << " ";
      cout << "(" << ir.didmap[did].len << ")" << endl;
      cout << "  ..." << sr.summary(q, did) << "..." << endl;
    }
    if (q->docs().size() > 5) {
      cout << "... (result trucated)" << endl;
    }
    
    
    if (ss.suggest(q))
      cout << "suggest query = " << q->tostring() << endl;
    cout << endl;
    delete q;
  }
  return 0;
}
