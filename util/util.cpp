#include "util.h"


#define BUF 1024
// Tokenize the whole file
void tokenize(string file, vector<string> &collect) {
    ifstream fin(file.c_str());
    char c[BUF+10];
    while(fin.getline(c, BUF, '\n')) {
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

// Collect regular file names recursively.
// Files found in 'exclude' are abandoned.
void collect(string path, vector<string> &files, set<string> &exclude) {
    struct dirent *entry;
    DIR *dp;

    dp = opendir(path.c_str());
    if (dp == NULL) {
        cerr<<"fail open file:"<<path<<endl;
        return;
    }
    while ((entry = readdir(dp))) {
        string name = string(entry->d_name);
        if (!name.length() || name[0]=='.') // ignore hidden or special files
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
void conjunct(vector<int>& a, vector<int>& b, vector<int>& c) { 
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
void disjunct(vector<int>& a, vector<int>& b, vector<int>& c) {
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
void diff(vector<int>& a, vector<int>& b, vector<int>& c) {
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

void dump(vector<int>& a) {
    for (unsigned i=0; i<a.size(); i++) {
        cout<<a[i]<<" ";
    }
    cout<<endl;
}
