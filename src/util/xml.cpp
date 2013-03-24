#include "util/xml.h"


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
  xmlDoc *doc = NULL;
  xmlNode *root = NULL;

  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION
  doc = xmlReadFile(file.c_str(), NULL, 0);
  if (doc == NULL) {
      cerr << "error: could not parse file" << file << endl;
  }
  root = xmlDocGetRootElement(doc);

  xmlwalk(doc, root, collect);

  xmlFreeDoc(doc);
  xmlCleanupParser();
}
