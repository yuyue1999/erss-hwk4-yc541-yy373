//
//  main.cpp
//  ECE568_HW4
//
//  Created by Talking_cat on 4/1/23.
//

#include <iostream>
#include <pqxx/pqxx>
#include <pthread.h>
#include <vector>
#include "socket.hpp"
#include "Temper.h"
#include <mutex>

std::mutex m_lock;
using namespace pqxx;
class Helper{
public:
    int client_fd;
    std::string ip;
    pqxx::connection* C;
    struct timespec *start;
    bool test=false;
    Helper(int temp_fd,std::string &temp_ip,pqxx::connection* tempC,struct timespec *startt,bool testt):
    client_fd(temp_fd),ip(temp_ip),C(tempC),start(startt),test(testt){}
};
void *handle(void* para){
    Helper * p = (Helper *) para;
    //pqxx::connection* C=p->C;
    int client_fd=p->client_fd;
    std::string ip=p->ip;
    size_t lengthXML = 0;
    //int len = recv(client_fd, (char*)&lengthXML, sizeof(size_t), MSG_WAITALL);
    //std::cout<<lengthXML<<std::endl;
    /*
    if (len <= 0 || lengthXML == 0) {
        close(client_fd);
        delete p;
        return nullptr;
	  }*/
    
    std::vector<char> store(65532,0);
    int size = recv(client_fd, store.data(), store.size(), 0);
    bool first=true;
    size_t temptemp=0;/*
    while(true){
      std::vector<char> temp(1,0);
      int size = recv(client_fd, temp.data(), temp.size(), 0);
      if(temp[0]=='\n'&& first){
        std::vector<char> tempstore=store;
        temptemp=store.size();
        tempstore.push_back('\0');
        std::string tempstring=tempstore.data();
        lengthXML=stoi(tempstring);
        first=false;
      }
      if(size==0){
        break;
      }
      for(int i=0;i<size;i++){
        store.push_back(temp[i]);
      }
      
      if(store.size()==lengthXML+temptemp+1){
        store.push_back('\0');
        break;
      }
    }*/

    std::string temprecv=(store.data());
    size_t findline=temprecv.find("\n");
    lengthXML=stoi(temprecv.substr(0,findline));
    std::string request=temprecv.substr(findline+1,lengthXML); 
    //std::string request(store.data());
    std::cout<<request<<std::endl;
    TEMP T;
    pqxx::connection *C = nullptr;
    while(true){
      try{
      C = new pqxx::connection("dbname=yy user=postgres password=passw0rd host=db");
      if (C->is_open()) {
        pqxx::work W (*C);
        W.exec("SET TRANSACTION ISOLATION LEVEL SERIALIZABLE");
        W.commit();
        break;
      } else {
        std::cerr << "Can't open database" << std::endl;
        continue;
      }
      } catch (const std::exception &e){
        continue;
      }
    }
    std::string respond = T.execute_request(request, C);
    C->disconnect();
    std::cout<<respond<<std::endl;
    size_t sendsize=respond.size();
    std::string fres=std::to_string(sendsize)+"\n"+respond;
    //send(client_fd,(char*)&sendsize,sizeof(size_t),0);
    //send(client_fd,respond.c_str(),respond.size(),0);
    send(client_fd,fres.c_str(),fres.size(),0);
    close(client_fd);
    if(p->test){
      struct timespec  end;
      clock_gettime(CLOCK_REALTIME, &end);
      double different = (1000000000.0 *(end.tv_sec - p->start->tv_sec) + end.tv_nsec - p->start->tv_nsec) / 1e9;
      std::cout << different << std::endl;
    }
    
    delete p;
    return nullptr;
}


int main(int argc, const char * argv[]) {
    Socket S;
    int socket_fd=S.BuildSocket();
    bool test=false;
    pqxx::connection *C;
    try{
      C = new connection("dbname=yy user=postgres password=passw0rd host=db");
      if (C->is_open()) {
        pqxx::work W (*C);
        W.exec("SET TRANSACTION ISOLATION LEVEL SERIALIZABLE");
        W.commit();

      } else {
        std::cout << "Can't open database" << std::endl;
        return 1;
      }
    } catch (const std::exception &e){
      std::cerr << e.what() << std::endl;
      return 1;
    }
    if(argc==2){
      test=true;
    }
    std::string delete1="DROP TABLE IF EXISTS TRANSAC_ORDER;";
    work Wd1(*C);
    Wd1.exec(delete1);
    Wd1.commit();
    std::string delete2="DROP TABLE IF EXISTS POSITION;";
    work Wd2(*C);
    Wd2.exec(delete2);
    Wd2.commit();
    std::string delete3="DROP TABLE IF EXISTS ACCOUNT;";
    work Wd3(*C);
    Wd3.exec(delete3);
    Wd3.commit();
    std::string delete4="DROP TABLE IF EXISTS SYMBOL;";
    work Wd4(*C);
    Wd4.exec(delete4);
    Wd4.commit();
    std::string sql1="CREATE TABLE SYMBOL (NAME TEXT PRIMARY KEY);";
    work Wc1(*C);
    Wc1.exec(sql1);
    Wc1.commit();
    std::string sql2="CREATE TABLE ACCOUNT (ACCOUNT_NUM INT PRIMARY KEY,BALANCE MONEY NOT NULL DEFAULT 0);";
      work Wc2(*C);
      Wc2.exec(sql2);
      Wc2.commit();
    std::string sql3="CREATE TABLE POSITION (ACCOUNT_NUM INT NOT NULL,SYMBOL TEXT NOT NULL,AMOUNT FLOAT NOT NULL DEFAULT 0,PRIMARY KEY (ACCOUNT_NUM, SYMBOL),FOREIGN KEY (ACCOUNT_NUM) REFERENCES ACCOUNT(ACCOUNT_NUM) ON DELETE CASCADE,FOREIGN KEY (SYMBOL) REFERENCES SYMBOL(NAME) ON DELETE CASCADE);";
      work Wc3(*C);
      Wc3.exec(sql3);
      Wc3.commit();
      std::string sql4="CREATE TABLE TRANSAC_ORDER (ID SERIAL PRIMARY KEY,TRANSAC_ID SERIAL NOT NULL,ACCOUNT_NUM INT NOT NULL,STATUS VARCHAR(100) NOT NULL,SYMBOL TEXT NOT NULL,AMOUNT FLOAT NOT NULL,PRICE MONEY NOT NULL,CREATE_TIME TIMESTAMPTZ NOT NULL DEFAULT now(),FOREIGN KEY (SYMBOL) REFERENCES SYMBOL(NAME) ON DELETE CASCADE,FOREIGN KEY (ACCOUNT_NUM) REFERENCES ACCOUNT(ACCOUNT_NUM) ON DELETE CASCADE);";
      work Wc4(*C);
      Wc4.exec(sql4);
      Wc4.commit();
      C->disconnect(); //Maybe some problems!!!!
      struct timespec start, end;
      clock_gettime(CLOCK_REALTIME, &start);
    while(true){
        struct sockaddr_storage connector_addr;
        socklen_t addr_len = sizeof(connector_addr);
        int client_fd= accept(socket_fd, (struct sockaddr *)&connector_addr, &addr_len);
        if (client_fd == -1) {
            std::cerr << "Cannot accept connection on that socket" << std::endl;
            continue;
        }
        std::string ip = inet_ntoa(((struct sockaddr_in *)&connector_addr)->sin_addr);
        Helper * parameter = new Helper(client_fd,ip,C,&start,test);
        pthread_t thread;
        pthread_create(&thread, NULL, handle, parameter);
        
	
    }
}
