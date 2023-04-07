#include"transact.hpp"

class creation{
public:
    Func F;
    std::string handle_create(const rapidxml::xml_node < > * node, pqxx::connection * C) {
    rapidxml::xml_document < > result_xml;
    rapidxml::xml_node < > * result_top = result_xml.allocate_node(rapidxml::node_element, "results");
    result_xml.append_node(result_top);

    rapidxml::xml_node < > * res_child_node;
    //pthread_mutex_lock(&mutex);

    for (; node != nullptr; node = node -> next_sibling()) {
      std::string root_str = node -> name();

      if (root_str == "account") {

        add_new_account(node, result_top, & result_xml, C);
      } else if (root_str == "symbol") {
        add_new_symbol(node, result_top, & result_xml, C);
      }
    }
    //pthread_mutex_unlock(&mutex);


    std::string res;
    rapidxml::print(std::back_inserter(res), result_xml, 0);
    return res;
  }
  void add_new_symbol(const rapidxml::xml_node < > * node, rapidxml::xml_node < > * result_top, rapidxml::xml_document < > * result_xml, pqxx::connection * C) {
    rapidxml::xml_attribute < > * attr = node -> first_attribute("sym");

    if (attr == nullptr) {
      std::cerr << "Create_Symbol: do not have sym\n";
      for (rapidxml::xml_node < > * it = node -> first_node("account"); it != nullptr; it = it -> next_sibling("account")) {
        rapidxml::xml_node < > * res_child_node = result_xml -> allocate_node(rapidxml::node_element, "error", "Do not have sym attr");
        res_child_node -> append_attribute(result_xml -> allocate_attribute("sym", ""));
        F.makeAtt(it, result_xml, res_child_node);
        result_top -> append_node(res_child_node);
      }
    } else {
      std::string sym(attr -> value());
      //handle symbol not alphanumeric
      if (!std::all_of(sym.begin(), sym.end(), [](char c) {
          return std::isalnum(c);
        })) {
        //std::cout << "[error] not alphanumeric!" << std::endl;

        for (rapidxml::xml_node < > * it = node -> first_node("account"); it != nullptr; it = it -> next_sibling("account")) {
          rapidxml::xml_node < > * res_child_node = result_xml -> allocate_node(rapidxml::node_element, "error", "sym not alphanumeric");
          char * allocated_sym = result_xml -> allocate_string(sym.c_str());
          res_child_node -> append_attribute(result_xml -> allocate_attribute("sym", allocated_sym));
          F.makeAtt(it, result_xml, res_child_node);
          result_top -> append_node(res_child_node);
        }

      } else {

        for (rapidxml::xml_node < > * child_node = node -> first_node("account"); child_node != nullptr; child_node = child_node -> next_sibling("account")) {
          add_new_position(sym, result_top, child_node, result_xml, C);
        }

      }

    }
  }
  void add_new_account(const rapidxml::xml_node < > * node, rapidxml::xml_node < > * result_top, rapidxml::xml_document < > * result_xml, pqxx::connection * C) {
    rapidxml::xml_node < > * res_node;

    rapidxml::xml_attribute < > * attr = node -> first_attribute("id");
    if (attr == nullptr) {
      //std::cerr << "Create_Account: do not have id\n";
      res_node = result_xml -> allocate_node(rapidxml::node_element, "error", "Do not have id attr");
      res_node -> append_attribute(result_xml -> allocate_attribute("id", ""));

    }
    std::string id(attr -> value());

    attr = node -> first_attribute("balance");
    if (attr == nullptr) {
      //std::cerr << "Create_Account: do not have balance\n";
      res_node = result_xml -> allocate_node(rapidxml::node_element, "error", "Do not have balance attr");
      char * id_ptr = result_xml -> allocate_string(id.c_str());
      res_node -> append_attribute(result_xml -> allocate_attribute("id", id_ptr));

    }
    std::string balance(attr -> value());

    pqxx::work W( * C);
    try {
      //pqxx::work W( * C);
      std::string query = "INSERT INTO ACCOUNT (ACCOUNT_NUM, BALANCE) VALUES(" + W.quote(id) + ", " + W.quote(balance) + ");";
      pqxx::result R = W.exec(query);
      W.commit();
      res_node = result_xml -> allocate_node(rapidxml::node_element, "created");
      char * id_ptr = result_xml -> allocate_string(id.c_str());
      res_node -> append_attribute(result_xml -> allocate_attribute("id", id_ptr));
    } catch (const pqxx::pqxx_exception & e) {
      W.abort();
      //std::cerr << "Database Error in <create> <account>: " << e.base().what() << std::endl;
      res_node = result_xml -> allocate_node(rapidxml::node_element, "error", "Can not create net account, account might already exist OR has invalid account or balance info");
      char * id_ptr = result_xml -> allocate_string(id.c_str());
      res_node -> append_attribute(result_xml -> allocate_attribute("id", id_ptr));

    }

    result_top -> append_node(res_node);
  }
  void add_new_position(std::string sym, rapidxml::xml_node < > * result_top, rapidxml::xml_node < > * node, rapidxml::xml_document < > * result_xml, pqxx::connection * C) {

    rapidxml::xml_node < > * res_node;

    rapidxml::xml_attribute < > * attr = node -> first_attribute("id");
    if (attr == nullptr) {
      //std::cerr << "Create_Symbol: do not have id\n";
      res_node = result_xml -> allocate_node(rapidxml::node_element, "error", "Do not have id attr");
      char * sym_ptr = result_xml -> allocate_string(sym.c_str());
      res_node -> append_attribute(result_xml -> allocate_attribute("sym", sym_ptr));
      res_node -> append_attribute(result_xml -> allocate_attribute("id", ""));

    }
    std::string id(attr -> value());

    std::string num(node -> value());

    if (num.size() == 0) {
      std::cerr << "Create_Symbol: do not have num\n";
      res_node = result_xml -> allocate_node(rapidxml::node_element, "error", "Do not have num");
      char * sym_ptr = result_xml -> allocate_string(sym.c_str());
      res_node -> append_attribute(result_xml -> allocate_attribute("sym", sym_ptr));
      F.makeAtt(node, result_xml, res_node);

    }
    int num_int;
    try {
      num_int = stoi(num);
      if (num_int <= 0) {
        throw std::exception();
      }
    } catch (const std::exception & e) {
      //std::cerr << "Do not have a valid NUM when creating a position.\n";
      res_node = result_xml -> allocate_node(rapidxml::node_element, "error", "Do not have a valid NUM when creating a position.");
      char * sym_ptr = result_xml -> allocate_string(sym.c_str());
      res_node -> append_attribute(result_xml -> allocate_attribute("sym", sym_ptr));
      F.makeAtt(node, result_xml, res_node);

    }

    std::string query;

    pqxx::work W( * C);
    try {
      query = "INSERT INTO SYMBOL VALUES (" + W.quote(sym) + ") ON CONFLICT(NAME) DO NOTHING; ";
      query += "INSERT INTO POSITION VALUES(" + W.quote(id) + ", " + W.quote(sym) + ", 0) ON CONFLICT(ACCOUNT_NUM, SYMBOL) DO NOTHING;";
      query += "UPDATE POSITION SET AMOUNT=AMOUNT+'" + std::to_string(num_int) + "' WHERE ACCOUNT_NUM=" + W.quote(id) + " AND SYMBOL=" + W.quote(sym) + ";";
      pqxx::result R = W.exec(query);

      W.commit();
      res_node = result_xml -> allocate_node(rapidxml::node_element, "created");
      char * sym_ptr = result_xml -> allocate_string(sym.c_str());
      res_node -> append_attribute(result_xml -> allocate_attribute("sym", sym_ptr));
      F.makeAtt(node, result_xml, res_node);
    } catch (const pqxx::pqxx_exception & e) {
      W.abort();
      std::cerr << "Database Error in <create> <symbol>: " << e.base().what() << std::endl;
      res_node = result_xml -> allocate_node(rapidxml::node_element, "error", "Position can not be created by database, might have invalid input id.");
      char * sym_ptr = result_xml -> allocate_string(sym.c_str());
      res_node -> append_attribute(result_xml -> allocate_attribute("sym", sym_ptr));
      F.makeAtt(node, result_xml, res_node);

    }

    result_top -> append_node(res_node);
  }
};
