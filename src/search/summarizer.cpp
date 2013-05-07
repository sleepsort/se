#include "search/summarizer.h"

const int Summarizer::MAX_RANGE = 75;
const char Summarizer::COLOR_RESET[] = "\033[00m";
const char Summarizer::COLOR_RED[]   = "\033[22;31m";
const char Summarizer::COLOR_GREEN[] = "\033[22;32m";
const char Summarizer::COLOR_BROWN[] = "\033[22;33m";
const char Summarizer::COLOR_BLUE[]  = "\033[22;34m";
const char Summarizer::COLOR_GREY[]  = "\033[01;30m";
const char Summarizer::COLOR_BOLD[]  = "\033[01;31m";


Summarizer::Summarizer(IndexReader &ir, FileLoader &fl) {
  this->ir = &ir;
  this->fl = &fl;
}
Summarizer::~Summarizer() {
}

void Summarizer::cleanbody(string &body) {
  for (unsigned i = 0; i < body.length(); i++) {
    if (!isgraph(body[i]) || isspace(body[i])) {
      body[i] = ' ';
    }
  }
}
void Summarizer::mark(const string &body, const vector<string> &query,
                      vector<pair<int, int> > &offsets) {
  string slice;
  int end = body.length(), from = 0, to = 0;
  while (to < end) {
    while (to < end && !isalnum(body[to]))
      to++;
    if (to >= end)
      break;
    from = to;
    while (to < end && isalnum(body[to]))
      to++;
    slice = body.substr(from, to - from);
    if (!nodigit(slice.c_str()) || to - from >= WORD_BUF) {
      from = to;
      continue;
    }
    lowercase(slice);
    porterstem(slice);
    if (find(query.begin(), query.end(), slice) != query.end()) {
      offsets.push_back(make_pair(from, to));
    }
    from = to;
  }
}
pair<int,int> Summarizer::maxspan(const vector<pair<int, int> > &offsets) {
  unsigned maxhit = 0, maxf = 0, maxt = MAX_RANGE-1;
  unsigned hit = 0, f = 0, t = 0;
  while (t < offsets.size()) {
    while (t < offsets.size()) {
      if (offsets[t].second - offsets[f].first > MAX_RANGE) {
        t--;
        break;
      }
      t++;
    }
    if (t == offsets.size()) {
      t--;
    }
    hit = t-f+1;
    if (hit > maxhit) {
      maxf = f;
      maxt = t;
      maxhit = hit;
    }
    f++;
    t++;
  }
  return make_pair(maxf, maxt);
}

pair<int, int> Summarizer::fixrange(const string &body, const pair<int, int> ori) {
  int f = ori.first;
  int t = ori.second - 1;
  int padding = MAX_RANGE - (t-f+1);
  int len = body.length();

  f -= padding / 2;
  t += padding / 2;

  if (f < 0) {
    t -= f;
    f = 0;
  } 
  if (t > len - 1) {
    f -= (t - len + 1);
    t = len - 1;
  }
  if (f < 0) {
    f = 0;
  }

  if (f > 0 && isalnum(body[f-1])) {
    while (f < ori.first && !isspace(body[f]))
      f++;
    while (f < ori.first && isspace(body[f]))
      f++;
  }
  if (t < len - 1 && isalnum(body[t+1])) {
    while (t > ori.second && !isspace(body[t]))
      t--;
    while (t > ori.second && isspace(body[t]))
      t--;
  }
  return make_pair(f, t+1);
}

string Summarizer::summary(Query *q, int did) {
  string body = "", snippet = "";
  vector<string> query;
  DocAttr attr;

  tokenize(q->tostring().c_str(), query);
  for (unsigned i = 0; i < query.size(); i++) {
    lowercase(query[i]);
    porterstem(query[i]);
  }

  attr = ir->didmap[did];
  fl->seek(attr.path, attr.offset);
  fl->body(body);

  cleanbody(body);

  return highlight(body, query);
}

string Summarizer::highlight(const string &body, const vector<string> &query) {
  vector<pair<int, int> > offsets;
  pair<int, int> span, range;
  string snippet = "";
  int i;

  mark(body, query, offsets);
  if (offsets.size() == 0) {
    offsets.push_back(make_pair(0,0));
  }

  span = maxspan(offsets);
  range.first = offsets[span.first].first;
  range.second = offsets[span.second].second;

  range = fixrange(body, range);

  i = span.first;
  snippet += COLOR_GREEN;
  snippet += body.substr(range.first, offsets[i].first-range.first);
  while (i < span.second) {
    snippet += COLOR_RED;
    snippet += body.substr(offsets[i].first, offsets[i].second-offsets[i].first);
    snippet += COLOR_GREEN;
    snippet += body.substr(offsets[i].second, offsets[i+1].first-offsets[i].second);
    i++;
  }
  snippet += COLOR_RED;
  snippet += body.substr(offsets[i].first, offsets[i].second-offsets[i].first);
  snippet += COLOR_GREEN;
  snippet += body.substr(offsets[i].second, range.second-offsets[i].second);
  snippet += COLOR_RESET;
  return snippet;
}
