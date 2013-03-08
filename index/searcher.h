#ifndef SEARCHER_H
#define SEARCHER_H
#include "reader.h"
#include "search/query.h"
#include "search/parser.h"
#include "util/util.h"

class IndexSearcher: public IndexReader {
public:
    IndexSearcher(string path);
    ~IndexSearcher();
    void search(Query *q);
    void report(Query *q);

private:
    vector<int> alldoc;
};
#endif
