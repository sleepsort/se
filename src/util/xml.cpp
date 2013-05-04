#include "util/xml.h"

void xmlbody(const string &file, string &str){
  str = "";
  pugi::xml_document doc;
  assert(doc.load_file(file.c_str()));
  pugi::xml_node tools = doc.child("newsitem");
  //tokenize(tools.child("title").child_value(), collect);
  //tokenize(tools.child("headline").child_value(), collect);
  str += tools.child("title").child_value();
  str += tools.child("headline").child_value();
  for (pugi::xml_node tool = tools.child("text").child("p");tool;
    tool = tool.next_sibling("p") ){
    //tokenize(tool.child_value(), collect);
    str += tool.child_value();
  }
}
