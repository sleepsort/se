#include <iostream>
#include <cassert>
#include "index/reader.h"
#include "search/suggester.h"
#include "util/util.h"
using namespace std;

// stupid tests

void testEditDistance() {
    assert(levendistance("dog", "do") == 1);
    assert(levendistance("cat", "cart") == 1 );
    assert(levendistance("cat", "cut") == 1 );
    assert(levendistance("cat", "act") == 2 );
}

void testSuggestion() {
    IndexReader ir("data/index/");
    Suggester sg(ir);
    vector<int> collect;
    string w = "protestatiions";
    sg.kgram(w, collect);
    sg.levenrank(w, collect);
    assert(collect.size() == 2);
}
int main(int argc, char **argv) {
    testEditDistance();
    testSuggestion();
    
    cout << "passed!" << endl;
    return 0;
}
