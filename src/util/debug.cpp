#include "util/debug.h"

void error(const char *format, ...) {
  va_list args;
  fprintf(stderr, "ERROR:");
  va_start (args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");
  exit(1);
}

deque<double> _TIME_START;
void tick() {
  timeval start;
  gettimeofday(&start, NULL);
  double t = start.tv_usec;
  t /= 1000000;
  t += start.tv_sec;
  _TIME_START.push_back(t);
}

void tock() {
  if (_TIME_START.empty())
    return;
  static string pad = "         ";
  timeval end;
  gettimeofday(&end, NULL);
  double t = end.tv_usec, v = _TIME_START.back();
  t /= 1000000;
  t += end.tv_sec;
  _TIME_START.pop_back();
  printf("%s%.3fs\n", pad.substr(0,_TIME_START.size()*2).c_str(), t-v);
}
