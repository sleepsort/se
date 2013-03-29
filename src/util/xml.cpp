#include "util/xml.h"

void xmltokenize(const string &file, vector<string> &collect){
    pugi::xml_document doc;
    assert(doc.load_file(file.c_str()));
    pugi::xml_node tools = doc.child("newsitem");
    tokenize(tools.child("title").child_value(), collect);
    tokenize(tools.child("headline").child_value(), collect);
    for (pugi::xml_node tool = tools.child("text").child("p");tool;
            tool = tool.next_sibling("p") ){
        tokenize(tool.child_value(), collect);
    }
}
