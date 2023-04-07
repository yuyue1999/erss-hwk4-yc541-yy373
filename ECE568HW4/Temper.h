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
#include "creat.hpp"


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
class TEMP{
public:
    std::string execute_request(std::string request, pqxx::connection * C) {
      creation cre;
      transtion trs;
      std::string empty_res_str;
      try {
        rapidxml::xml_document < > xml;
        xml.parse < 0 > (const_cast < char * > (request.data()));
        rapidxml::xml_node < > * req_root = xml.first_node();
        std::string root_str = req_root -> name();
        rapidxml::xml_document < > result_xml;
        rapidxml::xml_node < > * res_root = result_xml.allocate_node(rapidxml::node_element, "results");
        result_xml.append_node(res_root);
        rapidxml::print(std::back_inserter(empty_res_str), result_xml, 0);
        if (!req_root) {
          return empty_res_str;
        }
        if (root_str == "create") {
          rapidxml::xml_node < > * first_child = req_root -> first_node();
          if (first_child == 0) return "<results></results>";
          return cre.handle_create(static_cast <
            const rapidxml::xml_node < > * > (first_child), C);
        } else if (root_str == "transactions") {
          return trs.transact(req_root, C);
        }
        return empty_res_str;
      } catch (const pqxx::pqxx_exception & e) {
        std::cerr << "Database Error: " << e.base().what() << std::endl;
        return empty_res_str;
      } catch (std::exception & e) {
        std::cerr << "Parser Error: " << e.what() << std::endl;
        return empty_res_str;
      }
    }
};

