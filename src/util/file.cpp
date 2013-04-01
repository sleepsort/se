#include "util/file.h"

string extension(const string &filename) {
  int pos = filename.find_last_of(".");
  if (pos < 0) 
    return "";
  return filename.substr(pos + 1);
}

// Tokenize the whole file
void rawtokenize(const string &file, vector<string> &collect) {
  ifstream fin(file.c_str());
  char c[LINE_BUF+10];
  while (fin.getline(c, LINE_BUF, '\n')) {
    tokenize(c, collect);
  }
  fin.close();
}

// Collect regular file names recursively.
// Files found in 'exclude' are abandoned.
void collect(const string &path, vector<string> &files, set<string> &exclude) {
  struct dirent *entry;
  DIR *dp;
  if ((dp = opendir(path.c_str())) == NULL) {
    error("Util::fail open file: %s", path.c_str());
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
  set<string> sorted(files.begin(), files.end());
  files.clear();
  files.insert(files.begin(), sorted.begin(), sorted.end());

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
void fseekg(ifstream &fin, long long offset, ios_base::seekdir way) {
  fin.seekg(offset, way);
}
long long ftellg(ifstream &fin) {
  return fin.tellg();
}

void fwrite(ofstream &fout, void *buf, size_t len) {
  fout.write((char*)buf,len);
}
void fseekp(ofstream &fout, long long offset, ios_base::seekdir way) {
  fout.seekp(offset, way);
}
long long ftellp(ofstream &fout) {
  return fout.tellp();
}
