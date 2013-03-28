#include "util/array.h"


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


// insert key in fixed position
int array_insert(int* a, int len, int key, int pos) {
  if (pos < len && a[pos] == key)
    return -1;
  int j = len;
  while (j > pos) {
    a[j] = a[j-1];
    j--;
  }
  a[pos] = key;
  return pos;
}

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
  if (m < len && a[m] < key) {
    return m+1;
  } else {
    return m;
  }
}
