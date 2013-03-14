#include <iostream>
#include <cassert>
#include "util/util.h"
using namespace std;

// stupid tests

int main(int argc, char **argv) {

    // test edit distance
    assert( levendistance("dog","do") == 1);
    assert( levendistance("cat","cart") == 1 );
    assert( levendistance("cat","cut") == 1 );
    assert( levendistance("cat","act") == 2 );

    cout << "passed!" << endl;
    return 0;
}
