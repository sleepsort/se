#include <iostream>
#include "index/searcher.h" 

using namespace std;
int main(int argc, char **argv) {
    if (argc != 2) {
        cout << "usage: "<< argv[0] << " <index path>" <<endl;
        return 1;
    }
    string index_path = argv[1];

    IndexSearcher is(index_path);
    Parser p;
    string query;

    while (getline(cin,query)) {
        Query* q = p.parse(query);
        //q->dump(); cout<<endl;
        is.search(q);
        is.report(q);
        delete q;
    }
    return 0;
}
