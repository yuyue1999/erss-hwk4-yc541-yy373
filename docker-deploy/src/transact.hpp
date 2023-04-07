#include "helper.hpp"


class transtion{
public:
  Func F;
  std::string transact(rapidxml::xml_node<>* trans, pqxx::connection* C){
    rapidxml::xml_document<> respond;
    rapidxml::xml_node<>* root = respond.allocate_node(rapidxml::node_element, "results");
    respond.append_node(root);
    rapidxml::xml_attribute<> *target = trans->first_attribute("id");
    if(target!=0){
      std::string user(target->value());
      try{
        pqxx::nontransaction N(*C);
        std::string SQLreq="SELECT * FROM ACCOUNT WHERE ACCOUNT_NUM="+N.quote(user)+";";
        pqxx::result R(N.exec(SQLreq));
        N.commit();
        if (R.begin() == R.end()) {
          throw pqxx::argument_error("Cannot find user id");
        }
      }catch(const pqxx::pqxx_exception &e){
        for (rapidxml::xml_node<>* node = trans->first_node("order"); node != 0; node=node->next_sibling("order")) {
          rapidxml::xml_node<>* tempnode = respond.allocate_node(rapidxml::node_element, "error", "Invalid user_id attr123");
          root->append_node(tempnode);
          for (rapidxml::xml_attribute<> *attr = node->first_attribute(); attr != 0; attr=attr->next_attribute()) {
            char *name = respond.allocate_string(attr->name()); 
            char *value = respond.allocate_string(attr->value()); 
            tempnode->append_attribute(respond.allocate_attribute(name, value));
          }
        }
        std::string finalr;
        rapidxml::print(std::back_inserter(finalr), respond, 0);
        return finalr;
      }
        rapidxml::xml_node<>* child = trans->first_node(); 
        rapidxml::xml_node<>* ressultnode;
        while (true) {
          if(child==0){
            break;
          }
          std::string type(child->name());
          if (type.find("order")!=std::string::npos) {
            ressultnode = AddOrder(user, child, &respond, C);
          } else if (type.find("query")!=std::string::npos) {
            ressultnode = Query(user, child, &respond, C);
          } else if (type.find("cancel")!=std::string::npos) {
            ressultnode = Cancel(user, child, &respond, C);
          }
          if (ressultnode != nullptr) {
            root->append_node(ressultnode);
          }else if(ressultnode==nullptr){
            root->append_node(respond.allocate_node(rapidxml::node_element, "error", "Invalid transaction"));
          } 
          child = child->next_sibling();
        }
      std::string finalr;
      rapidxml::print(std::back_inserter(finalr), respond, 0);
      return finalr;
    }else{
      for (rapidxml::xml_node<>* node = trans->first_node("order"); node != 0; node=node->next_sibling("order")) {
          rapidxml::xml_node<>* tempnode = respond.allocate_node(rapidxml::node_element, "error", "Invalid input attibute");
          root->append_node(tempnode);
          for (rapidxml::xml_attribute<> *attr = node->first_attribute(); attr != 0; attr=attr->next_attribute()) {
            char *name = respond.allocate_string(attr->name()); 
            char *value = respond.allocate_string(attr->value()); 
            tempnode->append_attribute(respond.allocate_attribute(name, value));
          }
        }
      std::string finalr;
      rapidxml::print(std::back_inserter(finalr), respond, 0);
      return finalr;
    }
  }
  rapidxml::xml_node<>* Cancel(std::string user, rapidxml::xml_node<>* target, rapidxml::xml_document<>* document, pqxx::connection* C){
    rapidxml::xml_node<>* respond=document->allocate_node(rapidxml::node_element, "canceled");
    rapidxml::xml_attribute<> *attribute = target->first_attribute("id");
    if(attribute!=0){
      std::string trans_id(attribute->value());
      try{
      pqxx::nontransaction N1(*C);
      std::string SQLreq="SELECT * FROM TRANSAC_ORDER WHERE ACCOUNT_NUM="+N1.quote(user)+"AND TRANSAC_ID="+N1.quote(trans_id)+"AND STATUS='open';";
      pqxx::result R=N1.exec(SQLreq);
      N1.commit();
      if (R.begin() == R.end()) {
          throw pqxx::argument_error("Cannot find user id");
        }
      }catch(const pqxx::pqxx_exception &e){
        rapidxml::xml_node<>* tempnode = document->allocate_node(rapidxml::node_element, "error","Wrong trans_id");
        respond->append_node(tempnode);
        return respond;
      }
      std::string SQLreq;
      pqxx::work W(*C);
      char * temp_id = document->allocate_string(trans_id.c_str());
      respond->append_attribute(document->allocate_attribute("id", temp_id)); 
      SQLreq="UPDATE TRANSAC_ORDER SET STATUS='canceled', CREATE_TIME=now() WHERE TRANSAC_ID="+ W.quote(trans_id) +"AND ACCOUNT_NUM=" + W.quote(user) + "AND STATUS='open' RETURNING SYMBOL, AMOUNT, PRICE::numeric;";
      pqxx::result R2 = W.exec(SQLreq);
      std::string symbol= R2.begin()[0].as<std::string>();
      double amount= R2.begin()[1].as<double>();
      double price= R2.begin()[2].as<double>();
      if (amount < 0) { 
        amount=-amount;
        SQLreq = "UPDATE POSITION SET AMOUNT=AMOUNT+'" +std::to_string(amount) + "' WHERE ACCOUNT_NUM=" + W.quote(user) + "AND SYMBOL=" + W.quote(symbol) + ";";
      } else { 
        double total=amount*price;
        SQLreq= "UPDATE ACCOUNT SET BALANCE=BALANCE+'" + std::to_string(total) + "' WHERE ACCOUNT_NUM=" + W.quote(user) + ";";
      }
      W.exec(SQLreq);
      SQLreq= "SELECT STATUS, AMOUNT, PRICE::numeric, TO_CHAR(create_time, 'HH24:MI:SS') AS formatted_create_time FROM TRANSAC_ORDER WHERE TRANSAC_ID=" + W.quote(trans_id) + ";";
      pqxx::result R3(W.exec(SQLreq));
      W.commit();
      for (pqxx::result::const_iterator temp1 = R3.begin(); temp1 != R3.end(); ++temp1) {
        std::string status = temp1[0].as<std::string>();
        double tempa = temp1[1].as<double>();
        double tempp = temp1[2].as<double>();
        
        const char* chartime=temp1[3].as<const char*>();

        //int time = stoi(Stringt);
        if (status == "canceled") {
          rapidxml::xml_node<>* child = document->allocate_node(rapidxml::node_element, "canceled");
          respond->append_node(child);
          char * ch1 = document->allocate_string(std::to_string(abs(tempa)).c_str());
          child->append_attribute(document->allocate_attribute("shares", ch1));
          char * ch2 = document->allocate_string(chartime);
          child->append_attribute(document->allocate_attribute("time", ch2));
        } else if (status == "executed") {
          rapidxml::xml_node<>* child = document->allocate_node(rapidxml::node_element, "executed");
          respond->append_node(child);
          char * ch1 = document->allocate_string(std::to_string(abs(tempa)).c_str());
          child->append_attribute(document->allocate_attribute("shares", ch1));
          char * ch2 = document->allocate_string(std::to_string(tempp).c_str());
          child->append_attribute(document->allocate_attribute("price", ch2));
          char * ch3 = document->allocate_string(chartime);
          child->append_attribute(document->allocate_attribute("time", ch3));
        }
      }
      return respond;
    }
    return nullptr;
  }
  rapidxml::xml_node<>* Query(std::string user, rapidxml::xml_node<>* target, rapidxml::xml_document<>* document, pqxx::connection* C) {
    rapidxml::xml_node<>* res_node = document->allocate_node(rapidxml::node_element, "status");
    rapidxml::xml_attribute<> *attribute = target->first_attribute("id");
    if (attribute != 0) {
      std::string trans_id(attribute->value()); 
      char * id_ch = document->allocate_string(trans_id.c_str());
      res_node->append_attribute(document->allocate_attribute("id", id_ch)); 
      pqxx::nontransaction N(*C);
      pqxx::result R;
      try {
        //TO_CHAR(create_time, 'HH24:MI:SS') AS formatted_create_time
        std::string SQLreq=  "SELECT STATUS, AMOUNT, PRICE::numeric, TO_CHAR(CREATE_TIME, 'HH24:MI:SS') AS formatted_create_time FROM TRANSAC_ORDER WHERE TRANSAC_ID=" + N.quote(trans_id) + ";";
        R = N.exec(SQLreq);
        N.commit();
      }catch (const std::exception &e){
        //N.abort();
        return nullptr;
      }
      for (pqxx::result::const_iterator iter = R.begin(); iter != R.end(); ++iter) {
        std::string status = iter[0].as<std::string>();
        double tempa = iter[1].as<double>();
        double tempp = iter[2].as<double>();
        const char* chartime=iter[3].as<const char*>();
        if (status == "open") {
          rapidxml::xml_node<>* tempnode = document->allocate_node(rapidxml::node_element, "open");
          res_node->append_node(tempnode);
          char * ch1 = document->allocate_string(std::to_string(abs(tempa)).c_str());
          tempnode->append_attribute(document->allocate_attribute("shares", ch1));
        } else if (status == "canceled") {
          rapidxml::xml_node<>* tempnode = document->allocate_node(rapidxml::node_element, "canceled");
          res_node->append_node(tempnode);
          char * ch1 = document->allocate_string(std::to_string(abs(tempa)).c_str());
          tempnode->append_attribute(document->allocate_attribute("shares", ch1));
          char * ch2 = document->allocate_string(chartime);
          tempnode->append_attribute(document->allocate_attribute("time", ch2));
        } else if (status == "executed") {
          rapidxml::xml_node<>* tempnode = document->allocate_node(rapidxml::node_element, "executed");
          res_node->append_node(tempnode);
          char * ch1 = document->allocate_string(std::to_string(abs(tempa)).c_str());
          tempnode->append_attribute(document->allocate_attribute("shares", ch1));
          char * ch3 = document->allocate_string(std::to_string(tempp).c_str());
          tempnode->append_attribute(document->allocate_attribute("price", ch3));
          char * ch2 = document->allocate_string(chartime);
          tempnode->append_attribute(document->allocate_attribute("time", ch2));
        } 
      }
      return res_node;
    }else{
      return nullptr;
    }
}
rapidxml::xml_node<>* get_error_node(const char* error_msg, rapidxml::xml_node<>* node, rapidxml::xml_document<>* result_xml) {
  rapidxml::xml_node<>* res_node = result_xml->allocate_node(rapidxml::node_element, "error", error_msg);

  for (rapidxml::xml_attribute<>* attr = node->first_attribute(); attr != 0; attr = attr->next_attribute()) {
    char* name = result_xml->allocate_string(attr->name());
    char* value = result_xml->allocate_string(attr->value());
    res_node->append_attribute(result_xml->allocate_attribute(name, value));
  }

  return res_node;
}

rapidxml::xml_node<>* check_attr_error(const std::string& attr_name, rapidxml::xml_node<> *node, rapidxml::xml_document<> *result_xml, std::string &attr_value) {
    rapidxml::xml_attribute<> *attr = node->first_attribute(attr_name.c_str());
    if (attr == 0) {
        std::string error_msg = "transaction tag do not have " + attr_name + " attr";
        return get_error_node(error_msg.c_str(), node, result_xml);
    }
    attr_value.assign(attr->value());
    return nullptr;
}
int add_buy_order(const std::string &sym, const std::string &user_id, const std::string &amount, const std::string &limit, pqxx::work &W) {
  std::string sql = "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_NUM=" + W.quote(user_id) + " AND BALANCE>='" + std::to_string(std::stod(limit) * std::stod(amount)) + "';";
  pqxx::result R(W.exec(sql));

  if (R.begin() == R.end()) { 
    throw std::runtime_error("insufficient balance to buy");
  }

  sql = "INSERT INTO SYMBOL VALUES (" + W.quote(sym) + ") ON CONFLICT(NAME) DO NOTHING; ";
  sql += "UPDATE ACCOUNT SET BALANCE=BALANCE-'" + std::to_string(stod(limit) * stod(amount)) + "' WHERE ACCOUNT_NUM=" + W.quote(user_id) + "; ";
  sql += "INSERT INTO TRANSAC_ORDER(ACCOUNT_NUM, STATUS, SYMBOL, AMOUNT, PRICE) ";
  sql += "VALUES (" + W.quote(user_id) + ", 'open', " + W.quote(sym) + ", " + W.quote(amount) + ", " + W.quote(limit) + ") RETURNING TRANSAC_ID;";
  pqxx::result R1(W.exec(sql));
  W.commit();
  return R1.begin()[0].as<int>();
}

int add_sell_order(const std::string &sym, const std::string &user_id, const std::string &amount, const std::string &limit, pqxx::work &W) {
  std::stringstream ss;
  ss << "SELECT AMOUNT FROM POSITION WHERE ACCOUNT_NUM=" << W.quote(user_id) << " AND SYMBOL=" << W.quote(sym) << " AND AMOUNT>=" << std::abs(stod(amount)) << " FOR UPDATE;";
  
  pqxx::result R(W.exec(ss.str()));

  if (R.begin() == R.end()) { 
    throw std::runtime_error("insufficient balance to sell");
  }

  ss.str("");
  ss << "UPDATE POSITION SET AMOUNT=AMOUNT-'" << std::abs(stod(amount)) << "' WHERE ACCOUNT_NUM=" << W.quote(user_id) << " AND SYMBOL=" << W.quote(sym) << ";";
  ss << "INSERT INTO TRANSAC_ORDER(ACCOUNT_NUM, STATUS, SYMBOL, AMOUNT, PRICE) VALUES (" << W.quote(user_id) << ", 'open', " << W.quote(sym) << ", " << W.quote(amount) << ", " << W.quote(limit) << ") RETURNING TRANSAC_ID;" ; 
  pqxx::result R2(W.exec(ss.str()));
  W.commit();
  return R2.begin()[0].as<int>();
}


rapidxml::xml_node<>* AddOrder(std::string user_id, rapidxml::xml_node<>* node, rapidxml::xml_document<>* result_xml, pqxx::connection* C) {
  
rapidxml::xml_node<>* res_node;

std::string sym, amount, limit;
res_node = check_attr_error("sym", node, result_xml, sym);
if (res_node) {return res_node;}
res_node = check_attr_error("amount", node, result_xml, amount);
if (res_node) {return res_node;}
res_node = check_attr_error("limit", node, result_xml, limit);
if (res_node) {return res_node;}
  
  
  pqxx::work W(*C);
  
  
  try {

  int order_id;
  double amount_value=stod(amount);
  if(amount_value ==0){return get_error_node("transaction amount zero", node, result_xml);}
    if (amount_value < 0) { 
    try {
      order_id = add_sell_order(sym, user_id, amount, limit, W);
    } catch (const std::runtime_error &e) {
      W.abort();
      return get_error_node(e.what(), node, result_xml);
    }
  }

  else { 
    try {
      order_id = add_buy_order(sym, user_id, amount, limit, W);
    } catch (const std::runtime_error &e) {
      W.abort();
      return get_error_node(e.what(), node, result_xml);
    }
  }

    res_node = result_xml->allocate_node(rapidxml::node_element, "opened");
    for (rapidxml::xml_attribute < > * attr = node -> first_attribute(); attr != 0; attr = attr -> next_attribute()) {
      res_node -> append_attribute(result_xml -> allocate_attribute(result_xml -> allocate_string(attr -> name()), result_xml -> allocate_string(attr -> value())));
    }
    char * order_id_char = result_xml->allocate_string(std::to_string(order_id).c_str()); 
    res_node->append_attribute(result_xml->allocate_attribute("id", order_id_char)); 
    //return res_node;
    
    try {
      Match(order_id, C);
      
      return res_node;
    }
    catch (const pqxx::pqxx_exception & e) {
      //std::cerr << "Match order error: " << e.base().what() << std::endl;
      return res_node;
    }
    
  }
  catch (const std::exception &e){
    W.abort();
    return get_error_node("invalid order", node, result_xml);
  }
  catch (const pqxx::pqxx_exception & e) {
    W.abort();
    return get_error_node("order can not execute by database", node, result_xml);
  }
  
}


void get_self_info(int flag, pqxx::result R, int * sell_id, std::string * symbol, double * sell_amount, double * sell_price, int * sell_account, std::string * sell_time){

*sell_id = R.begin()[0].as<int>();
*symbol = R.begin()[1].as<std::string>();
if(flag==1){
  *sell_amount = -R.begin()[2].as<double>();
}
else if (flag==2){
  *sell_amount = R.begin()[2].as<double>();
}

*sell_price = R.begin()[3].as<double>();
*sell_account = R.begin()[4].as<int>();
*sell_time = R.begin()[5].as<std::string>();


}


void Match(int order_id, pqxx::connection* C) {
  
  int sell_id, sell_account,sell_order;
  int buy_id, buy_account, buy_order;
  double sell_amount, sell_price;
  double buy_amount,buy_price;
  
  std::string symbol,sell_time, buy_time;

  
  while (true) {
    std::stringstream ss;
    
    pqxx::work W(*C);
    ss << "SELECT ID, SYMBOL, AMOUNT, PRICE::numeric, ACCOUNT_NUM, EXTRACT(EPOCH FROM CREATE_TIME) FROM TRANSAC_ORDER WHERE TRANSAC_ID=" << order_id << " AND STATUS='open' FOR UPDATE;";
    pqxx::result R = W.exec(ss.str());
    
    if (R.begin() == R.end()) {
      W.abort();
      return;
    }
    
    
    if (R.begin()[2].as<double>() > 0) { // seller
      get_self_info(2,R,  &buy_id, &symbol, &buy_amount, &buy_price,&buy_account, &buy_time);

      buy_order = order_id;
      
      
      ss.str("");
      ss << "SELECT ID, SYMBOL, TRANSAC_ID, AMOUNT, PRICE::numeric, ACCOUNT_NUM ,EXTRACT(EPOCH FROM CREATE_TIME) FROM TRANSAC_ORDER WHERE STATUS='open' AND SYMBOL="<< W.quote(symbol) << " AND AMOUNT<0 AND PRICE<='" << buy_price << "' ORDER BY PRICE ASC, CREATE_TIME ASC LIMIT 1 FOR UPDATE;";
      pqxx::result R2 = W.exec(ss.str());
      if (R2.begin() == R2.end()) {
        W.abort();
        return;
      }
      
      sell_id = R2.begin()[0].as<int>();
      symbol = R2.begin()[1].as<std::string>();
      sell_order = R2.begin()[2].as<int>();
      sell_amount = -R2.begin()[3].as<double>();
      sell_price = R2.begin()[4].as<double>();
      sell_account = R2.begin()[5].as<int>();
      sell_time = R2.begin()[6].as<std::string>(); 
      
      
      
    } 
    
    else { // buyer
      get_self_info(1,R,  &sell_id, &symbol, &sell_amount, &sell_price,&sell_account, &sell_time);
      sell_order = order_id;
      ss.str("");
      ss << "SELECT ID, SYMBOL, TRANSAC_ID, AMOUNT, PRICE::numeric, ACCOUNT_NUM, EXTRACT(EPOCH FROM CREATE_TIME) FROM TRANSAC_ORDER WHERE STATUS='open' AND SYMBOL="<< W.quote(symbol) << " AND AMOUNT>0 AND PRICE>='" << sell_price << "' ORDER BY PRICE DESC, CREATE_TIME ASC LIMIT 1 FOR UPDATE;";
      pqxx::result R2 = W.exec(ss.str());
      if (R2.begin() == R2.end()) {
        W.abort();
        return;
      }
      /////
      buy_id = R2.begin()[0].as<int>();
      symbol = R2.begin()[1].as<std::string>();
      buy_order = R2.begin()[2].as<int>();
      buy_amount = R2.begin()[3].as<double>();
      buy_price = R2.begin()[4].as<double>();
      buy_account = R2.begin()[5].as<int>();
      buy_time = R2.begin()[6].as<std::string>(); 
      
    ////
      
      
    }
    
    // execute
    double amount = sell_amount < buy_amount ? sell_amount : buy_amount;
    double price = stod(sell_time) < stod(buy_time) ? sell_price : buy_price;
    
    // for buyer: create position(if nessecery); add sym; possibly refund; change order status; possiblely split order
    //1 buy update sym
    ss.str("");
    ss << "INSERT INTO POSITION VALUES(" << buy_account << ", " << W.quote(symbol) << ", 0) ON CONFLICT(ACCOUNT_NUM, SYMBOL) DO NOTHING;";
    ss << "UPDATE POSITION SET AMOUNT=AMOUNT+'" << amount << "' WHERE ACCOUNT_NUM=" << buy_account << " AND SYMBOL=" << W.quote(symbol) << ";";
    
    //sell update sym:already done before
    //but need update if amount different
    //ss << "UPDATE POSITION SET AMOUNT=AMOUNT+'" << (sell_amount-amount)<< "' WHERE ACCOUNT_ID=" << sell_account << " AND SYMBOL=" << W.quote(symbol) << ";";
    
    //2 buy update account balance: already done before
    //but need update if price different 
    
    ss << "UPDATE ACCOUNT SET BALANCE=BALANCE+'" << (buy_price - price) * amount << "' WHERE ACCOUNT_NUM=" << buy_account << ";";

    //sell update account balance
    ss << "UPDATE ACCOUNT SET BALANCE=BALANCE+'" << price * amount << "' WHERE ACCOUNT_NUM=" << sell_account << ";";
    //std::cout<<std::endl<<ss.str()<<std::endl<<std::endl;
    pqxx::result update_accoutnt_position(W.exec(ss.str()));
    
    
    //for amount
    ss.str("");
    if(amount<sell_amount){ //sell1 part, buy2 all
    
    //buy update to exe
    ss << "UPDATE TRANSAC_ORDER SET STATUS='executed', AMOUNT=" << amount << ", PRICE=" << price << ", CREATE_TIME=now() WHERE ID=" << buy_id << ";";
    //sell update to exe, use exe amount & exe price
    ss << "UPDATE TRANSAC_ORDER SET STATUS='executed', AMOUNT=" << amount << ", PRICE=" << price << ", CREATE_TIME=now() WHERE ID=" << sell_id << ";";
    //sell insert new open
    ss << "INSERT INTO TRANSAC_ORDER(TRANSAC_ID, ACCOUNT_NUM, STATUS, SYMBOL, AMOUNT, PRICE, CREATE_TIME) VALUES (" << sell_order << ", " << sell_account<< ", 'open', " << W.quote(symbol) << ", " << -(sell_amount - amount) << ", " << sell_price << ", to_timestamp(" << sell_time << "));";
    
    }
    else if(amount<buy_amount){//sell all, buy part
    
    //sell update to exe
    ss << "UPDATE TRANSAC_ORDER SET STATUS='executed', AMOUNT=" << amount << ", PRICE=" << price << ", CREATE_TIME=now() WHERE ID=" << sell_id << ";";
    //buy update to exe, use exe amount & exe price
    ss << "UPDATE TRANSAC_ORDER SET STATUS='executed', AMOUNT=" << amount << ", PRICE=" << price << ", CREATE_TIME=now() WHERE ID=" << buy_id << ";";
    //buy insert new open
    ss << "INSERT INTO TRANSAC_ORDER(TRANSAC_ID, ACCOUNT_NUM, STATUS, SYMBOL, AMOUNT, PRICE, CREATE_TIME) VALUES (" << buy_order << ", " << buy_account<< ", 'open', " << W.quote(symbol) << ", " << buy_amount - amount << ", " << buy_price << ", to_timestamp(" << buy_time << "));";
    }
    else{
    
    //std::cout<<"!!!!!exe the exact amount, exit match"<<std::endl;
    //sell update to exe
    ss << "UPDATE TRANSAC_ORDER SET STATUS='executed', PRICE=" << price << ", AMOUNT=" << amount << ", CREATE_TIME=now() WHERE ID=" << sell_id << ";";
    //buy update to exe, use exe amount & exe price
    ss << "UPDATE TRANSAC_ORDER SET STATUS='executed', PRICE=" << price << ", AMOUNT=" << amount << ", CREATE_TIME=now() WHERE ID=" << buy_id << ";";
    pqxx::result update_amount(W.exec(ss.str()));
    W.commit();
    return;
    }
    pqxx::result update_amount(W.exec(ss.str()));
    W.commit();
    
    
  }
}


};