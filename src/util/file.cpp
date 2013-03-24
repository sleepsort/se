#include "util/file.h"

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

void fread(ifstream &fin, void *buf, size_t len) {
  fin.read((char*)buf, len);
}
// get the next buf while keeping original file pointer
void fpeek(ifstream &fin, void *buf, size_t len) {
  long long fp = fin.tellg();
  fin.read((char*)buf, len);
  fin.seekg(fp, ios::beg);
}
void fwrite(ofstream &fout, void *buf, size_t len) {
  fout.write((char*)buf,len);
}
