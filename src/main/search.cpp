#include <iostream>
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

  IndexReader ir(indx_path);
  IndexSearcher is(ir);
  Suggester ss(ir);
  Parser p;
  string query;

  while (getline(cin, query)) {
    Query* q = p.parse(query);
    is.search(q);
    is.report(q);
    if (ss.suggest(q))
    cout << "suggest query = " << q->tostring() << endl;
    cout << endl;
    delete q;
  }
  return 0;
}
