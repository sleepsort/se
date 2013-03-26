#include "util/xml.h"
#include <malloc.h>

void xmlwalk(xmlDoc *doc, xmlNode *node, vector<string> &collect) {
  for (xmlNode *cur = node ; cur; cur = cur->next) {
    if (cur->type == XML_ELEMENT_NODE && 
       !xmlStrcmp(cur->name, (xmlChar *)"text")) {
      for (xmlNode *next = cur->children; next; next = next->next) {
        xmlChar* line = xmlNodeGetContent(next);
        tokenize((char*)line, collect);
        xmlFree(line);
      }
    }
    xmlwalk(doc, node->children, collect);
  }
}

void xmltokenize(const string &file, vector<string> &collect) {
  static int x = 0;
  if (!x) {
    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION
  }

  xmlDoc *doc = NULL;
  xmlNode *root = NULL;

  doc = xmlReadFile(file.c_str(), NULL, 0);
  if (doc == NULL) {
      cerr << "error: could not parse file" << file << endl;
  }
  root = xmlDocGetRootElement(doc);

  xmlwalk(doc, root, collect);
  xmlFreeDoc(doc);

  x += 1;
  if (x > 10000) {
    xmlCleanupParser();
    /* 
     * This stupid library will allocate much tiny pieces of 
     * memory, which will be freed but not returned to kernel.
     * Which, will cause a fake memory leak. Here, we force 
     * the kernel to accept those pieces.
     */
    malloc_trim(0);
    x = 0;
  }
}
