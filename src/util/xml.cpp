#include "util/xml.h"
using tinyxml2::XMLElement;
using tinyxml2::XMLDocument;

void xmlwalk(XMLElement *node, vector<string> &collect) {
  XMLElement *next = node->FirstChildElement();
  while (next) {
    if (!strcmp(next->Name(), "text")) {
      XMLElement *para = next->FirstChildElement();
      while (para) {
        tokenize(para->GetText(), collect);
        para = para->NextSiblingElement();
      }
      return;
    }
    xmlwalk(next, collect);
    next = next->NextSiblingElement();
  }
}

void xmltokenize(const string &file, vector<string> &collect) {
  XMLDocument doc;
  doc.LoadFile(file.c_str());
  xmlwalk(doc.RootElement(), collect);
}
