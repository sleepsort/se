#include <cmath>
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

    IndexReader ir(indx_path);
    IndexSearcher is(ir);
    Scorer sc(ir);
    Parser p;
    string query;
    int padding = log10(ir.didmap.size())+1;

    while (getline(cin, query)) {
        Query* q = p.parse(query);
        is.search(q);
        sc.init(q);
        vector<pair<int, double> > &result = sc.score();
        cout << "query = " << q->tostring() << endl;
        cout << "numhit = " << result.size() << endl;
        cout << "hitdocs = " << endl;
        for (unsigned i = 0; i < result.size() && i < 10; i++) {
          int did = result[i].first;
          cout << "[" << setw(padding) << setfill(' ') << did << "] ";
          cout << fixed << setprecision(6) << result[i].second << " ";
          cout << ir.didmap[did].name << endl;
        }
        cout << endl;
        if (result.size() > 10) {
          cout << "... (result trucated)" << endl;
        }
        delete q;
    }
    return 0;
}
