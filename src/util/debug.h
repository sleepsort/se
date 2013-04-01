#ifndef UTIL_DEBUG_H_
#define UTIL_DEBUG_H_
#include <sys/time.h>
#include <cstdarg>
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <string>
#include <deque>
using namespace std;

void error(const char *format, ...);
void tick();
void tock();

#endif  // UTIL_DEBUG_H_
