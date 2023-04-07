#include "ref/rapidxml_ext.hpp"
#include <string>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <sstream>
#include <fstream>
#include <iostream>
#include <pqxx/pqxx>
#include <algorithm>
#include <pthread.h>


class Func{
public:
  void makeAtt(rapidxml::xml_node < > * node, rapidxml::xml_document < > * result_xml, rapidxml::xml_node < > * res_node) {
    for (rapidxml::xml_attribute < > * attr = node -> first_attribute(); attr != 0; attr = attr -> next_attribute()) {
      char * name = result_xml -> allocate_string(attr -> name());
      char * value = result_xml -> allocate_string(attr -> value());
      res_node -> append_attribute(result_xml -> allocate_attribute(name, value));
    }
  }

};