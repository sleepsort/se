#include "util/util.h"


// Tokenize the whole file
void tokenize(const string &file, vector<string> &collect) {
  ifstream fin(file.c_str());
  char c[LINE_BUF+10];
  while (fin.getline(c, LINE_BUF, '\n')) {
    int upto = 0, cur, sz = strlen(c);
    while (upto < sz) {
      while (upto < sz && !isalnum(c[upto]))
        upto++;
      if (upto >= sz)
        break;
      cur = upto;
      while (upto < sz && isalnum(c[upto]))
        upto++;
      c[upto] = '\0';
      collect.push_back(&c[cur]);
    }
  }
}

// simply make all character lowercased
void lowercase(string &t) {
  transform(t.begin(), t.end(), t.begin(), ::tolower);
}

// Using porter algorithm to stem word s
// input should make sure all words are lowercased
// TODO(billy): use snprintf instead of strcpy
void porterstem(string &s) {
  char t[WORD_BUF+10] = {0};
  strcpy(t, s.c_str());
  t[stem(t, 0, s.length()-1)+1] = '\0';
  s.replace(0, s.length(), t);
}


/*
 * String style "itoa":
 * Written by Oromis
 */
string itoa(int value) {
    if (value == 0)
        return "0";
    std::string temp="";
    std::string returnvalue="";
    while (value>0)
    {
        temp+=value%10+48;
        value/=10;
    }
    for (unsigned int i=0;i<temp.length();i++)
        returnvalue+=temp[temp.length()-i-1];
    return returnvalue;
}

int min(int a1, int a2, int a3) {
  if (a1 < a2) {
    return a1 < a3 ? a1 : a3;
  } else {
    return a2 < a3 ? a2 : a3;
  }
}

// edit distance between two strings
// it is assumed that add,delete,replace share the same weight
int levendistance(const string &s1, const string &s2) {
  int d[WORD_BUF][WORD_BUF] = { {0} };
  int sz1 = s1.length(), sz2 = s2.length();
  if (sz1 == 0 || sz2 == 0)
    return -1;
  for (int i = 0; i <= sz1; ++i)
    d[i][0] = i;
  for (int j = 0; j <= sz2; ++j)
    d[0][j] = j;
  for (int j = 1; j <= sz2; ++j) {
    for (int i = 1; i <= sz1; ++i) {
      if (s1[i-1] == s2[j-1]) {
        d[i][j] = d[i-1][j-1];
      } else {
        d[i][j] = min(d[i-1][j], d[i][j-1], d[i-1][j-1]) + 1;
      }
    }
  }
  return d[sz1][sz2];
}

// Collect regular file names recursively.
// Files found in 'exclude' are abandoned.
void collect(const string &path, vector<string> &files, set<string> &exclude) {
  struct dirent *entry;
  DIR *dp;

  dp = opendir(path.c_str());
  if (dp == NULL) {
    cerr << "Util::fail open file:" << path << endl;
    return;
  }
  // TODO(billy): readdir_r should be thread safe to replace this
  while ((entry = readdir(dp))) {
    string name = string(entry->d_name);
    if (!name.length() || name[0] == '.')  // ignore hidden or special files
      continue;
    if (exclude.find(name) != exclude.end())
      continue;
    name = path+"/"+name;
    if (entry->d_type == DT_DIR) {
      collect(name, files, exclude);
    } else {
      files.push_back(name);
    }
  }
  closedir(dp);
  return;
}


// C = A AND B
// also safe for A = A AND B
void conjunct(vector<int> &a, vector<int> &b, vector<int> &c) {
  int l = 0, r = 0;
  int sa = a.size(), sb = b.size(), sc = c.size();
  while (l < sa && r < sb) {
    if (a[l] == b[r]) {
      c.push_back(a[l]);
      l++; r++;
    } else if (a[l] < b[r]) {
      l++;
    } else {
      r++;
    }
  }
  c.erase(c.begin(), c.begin()+sc);
}

// C = A OR B
// also safe for A = A OR B
void disjunct(vector<int> &a, vector<int> &b, vector<int> &c) {
  int l = 0, r = 0;
  int sa = a.size(), sb = b.size(), sc = c.size();
  while (l < sa && r < sb) {
    if (a[l] == b[r]) {
      c.push_back(a[l]);
      l++; r++;
    } else if (a[l] < b[r]) {
      c.push_back(a[l]);
      l++;
    } else {
      c.push_back(b[r]);
      r++;
    }
  }
  if (l < sa) {
    c.insert(c.end(), a.begin()+l, a.begin()+sa);
  } else if (r < sb) {
    c.insert(c.end(), b.begin()+r, b.begin()+sb);
  }
  c.erase(c.begin(), c.begin()+sc);
}

// C = A - B
// also safe for A = A - B
void diff(vector<int> &a, vector<int> &b, vector<int> &c) {
  int l = 0, r = 0;
  int sa = a.size(), sb = b.size(), sc = c.size();
  while (l < sa && r < sb) {
    while (l < sa && r < sb && a[l] == b[r]) {
      l++; r++;
    }
    while (l < sa && r < sb && a[l] != b[r]) {
      c.push_back(a[l]);
      l++;
    }
  }
  if (l < sa) {
    c.insert(c.end(), a.begin()+l, a.begin()+sa);
  }
  c.erase(c.begin(), c.begin()+sc);
}

// C = A OR B
// also safe for A = A OR B
// ( elements are: <freq, id> )
void disjunct(vector<pair<int, int> > &a, vector<pair<int, int> > &b,
              vector<pair<int, int> > &c) {
  int l = 0, r = 0;
  int sa = a.size(), sb = b.size(), sc = c.size();
  while (l < sa && r < sb) {
    if (a[l].second == b[r].second) {
      a[l].first += b[r].first;
      c.push_back(a[l]);
      l++; r++;
    } else if (a[l].second < b[r].second) {
      c.push_back(a[l]);
      l++;
    } else {
      c.push_back(b[r]);
      r++;
    }
  }
  if (l < sa) {
    c.insert(c.end(), a.begin()+l, a.begin()+sa);
  } else if (r < sb) {
    c.insert(c.end(), b.begin()+r, b.begin()+sb);
  }
  c.erase(c.begin(), c.begin()+sc);
}

void dump(vector<int> &a) {
  for (unsigned i = 0; i < a.size(); ++i) {
    cout << a[i] << " ";
  }
  cout << endl;
}
void dump(vector<pair<int, int> > &a) {
  for (unsigned i = 0; i < a.size(); ++i) {
    cout << " <" << a[i].first  <<  ","  << a[i].second << "> ";
  }
  cout << endl;
}



// insert key to fit strict ascending order
int array_insert(int* a, int len, int key) {
  if (len == 0) {
    a[0] = key;
    return 0;
  }
  int i = 0, j = 0;
  j = i = bsearch(a, len, key);
  if (i < len && a[i] == key) {   // duplicate
    return -1;
  }
  j = len;
  while (j > i) {
    a[j] = a[j-1];
    j--;
  }
  a[i] = key;
  return i;
}

// insert key in fixed position
// the array is unordered
void array_insert(int* a, int len, int key, int pos) {
  int j = len;
  if (a[pos] == key)
    return;
  while (j > pos) {
    a[j] = a[j-1];
    j--;
  }
  a[pos] = key;
}


// binary search, return the position
// of found key, or appropriate position
// to insert while keeping ascending order
int bsearch(const int*a, int len, int key) {
  int f = 0, t = len -1, m = 0;
  while (f <= t) {
    m = (f+t)/2;
    if (a[m] == key) {
      return m;
    } else if (a[m] < key) {
      f = m+1;
    } else if (a[m] > key) {
      t = m-1;
    }
  }
  if (a[m] < key) {
    return m+1;
  } else {
    return m;
  }
}
