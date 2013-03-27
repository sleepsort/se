#ifndef UTIL_PORTER_H_
#define UTIL_PORTER_H_
/* This is the Porter stemming algorithm, coded up in ANSI C by the
   author. It may be be regarded as cononical, in that it follows the
   algorithm presented in

   Porter, 1980, An algorithm for suffix stripping, Program, Vol. 14,
   no. 3, pp 130-137,

   only differing from it at the points maked --DEPARTURE-- below.

   The algorithm as described in the paper could be exactly replicated
   by adjusting the points of DEPARTURE, but this is barely necessary,
   because (a) the points of DEPARTURE are definitely improvements, and
   (b) no encoding of the Porter stemmer I have seen is anything like
   as exact as this version, even with the points of DEPARTURE!

   You can compile it on Unix with 'gcc -O3 -o stem stem.c' after which
   'stem' takes a list of inputs and sends the stemmed equivalent to
   stdout.

   The algorithm as encoded here is particularly fast.
*/

#include <memory.h>
#include <cstdlib>
#include <string>
using std::string;

#define TRUE 1
#define FALSE 0

int cons(int i);
int m();
int vowelinstem();
int doublec(int j);
int cvc(int i);
int ends(const string& s);
void setto(const string &s);
void r(const string & s);
void step1ab();
void step1c();
void step2();
void step3();
void step4();
void step5();
int stem(char * p, int i, int j);

/*--------------------stemmer definition ends here------------------------*/
#endif  // UTIL_PORTER_H_
