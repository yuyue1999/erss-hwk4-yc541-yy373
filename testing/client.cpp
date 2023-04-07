//
//  client.cpp
//  ECE568_HW4
//
//  Created by Talking_cat on 4/3/23.
//
#include <vector>
#include<cstdlib>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sstream>
#include <pthread.h>
#include "socket.hpp"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#define PORT "12345"


int connectTo(std::string &ip){
    int socketfd;
    struct addrinfo hints, *servinfo;
    int rv;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(ip.c_str(), PORT, &hints, &servinfo)) != 0) {
        std::cerr << "Cannot get address information" <<std::endl;
        return EXIT_FAILURE;
    }
    
    socketfd = socket(servinfo->ai_family,
                      servinfo->ai_socktype,
                      servinfo->ai_protocol);
    if (socketfd == -1) {
        std::cerr << "Cannot create socket" << std::endl;
        return EXIT_FAILURE;
    }
    int server_fd = connect(socketfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if(server_fd == -1){
        std::cerr<<"Cannot connect"<<std::endl;
        return EXIT_FAILURE;
    }
    freeaddrinfo(servinfo);
    return socketfd;
}


/*
int main(int argc, char ** argv){
    if (argc != 2) {
        std::cout << "./client host_name\n";
        return -1;
      }
      const char * host_name = argv[1];
    std::string ip(host_name);
    int times=200;
    Socket S;
    for(int i=0;i<times;i++){
    int socketfd=connectTo(ip);
    std::string request;
    request="<create><account id=\""+std::to_string(i+10)+"\" balance=\"1000000\"/><symbol sym=\""+std::to_string(i+10)+"\"><account id=\""+std::to_string(i+10)+"\">1000</account></symbol></create>";
    size_t size = request.size();
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, request.c_str(), request.size(), 0);
    //std::cout<<cat<<std::endl;
    size_t length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    std::vector<char> store=S.receiveall(socketfd,length);
    std::string respond(store.data());
    std::cout<<respond<<std::endl;
    close(socketfd);
    }
    //sleep(5);
    
    srand ((unsigned int)time(NULL));
    int limit_low = 50;
    int limit_high = 100;
    int amount_low = -20;
    int amount_high = -1;
    for (size_t i = 0; i < times; ++i) {
    int socketfd=connectTo(ip);
    std::string id = std::to_string(i+10);
    std::string sym = std::to_string(i+10);
    //std::string limit = std::to_string(rand() % (limit_high - limit_low) + limit_low);
    //std::string amount = std::to_string(rand() % (amount_high - amount_low) + amount_low);
    std::string amount = std::to_string(-10);
    std::string limit = std::to_string(10);
    std::string req= "<transactions id=\"" + id + "\"><order sym=\"" + sym +"\" amount=\"" + amount + "\" limit=\"" + limit + "\"/> </transactions>";
    size_t size = req.size();
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, req.c_str(), req.size(), 0);
    size_t length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    std::vector<char> store=S.receiveall(socketfd,length);
    std::string respond(store.data());
    std::cout<<respond<<std::endl;
    
    close(socketfd);
  }
    int amount_low1 = 1;
    int amount_high1 = 20;
    for (size_t i = 0; i < times; ++i) {
    int socketfd=connectTo(ip);
    std::string id = std::to_string(i+10);
    std::string sym = std::to_string(rand()%times+10);
    //std::string limit = std::to_string(rand() % (limit_high - limit_low) + limit_low);
    //std::string amount = std::to_string(rand() % (amount_high1 - amount_low1) + amount_low1);
    std::string amount = std::to_string(5);
    std::string limit = std::to_string(10);
    std::string req= "<transactions id=\"" + id + "\"><order sym=\"" + sym +"\" amount=\"" + amount + "\" limit=\"" + limit + "\"/> </transactions>";
    size_t size = req.size();
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, req.c_str(), req.size(), 0);
    size_t length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    std::vector<char> store=S.receiveall(socketfd,length);
    std::string respond(store.data());
    std::cout<<respond<<std::endl;
    
    close(socketfd);
  }
 
  //sleep(5);
  for (size_t i = 0; i < times; ++i) {
    int socketfd=connectTo(ip);
    //std::stringstream ss;
    //std::string id = std::to_string(rand() % times+10);
    std::string id=std::to_string(i+10);
    //std::string req= "<transactions id=\"" + id + "\"><cancel id=\"" + id +"\"/><query id=\"" + std::to_string(i+1) +"\"/> </transactions>";
    std::string req= "<transactions id=\"" + id + "\"><query id=\"" + std::to_string(i+1) +"\"/> <cancel id=\"" + std::to_string(i+1) +"\"/><query id=\"" + std::to_string(i+1) +"\"/></transactions>";
    //std::cout << "request:\n" << req << "\n";
    size_t size = req.size();
    //std::cout<<size<<std::endl;
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, req.c_str(), req.size(), 0);
  
    size_t length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    std::vector<char> store=S.receiveall(socketfd,length);
    std::string respond(store.data());
    std::cout<<respond<<std::endl;
    
    close(socketfd);
  }
  
    return 1;
    
    
    
}*/


int main(int argc, char ** argv){
    if (argc != 2) {
        std::cout << "./client host_name\n";
        return -1;
      }
      const char * host_name = argv[1];
    std::string ip(host_name);
    int times=10;
    Socket S;
    //101 102 103 104 105 106
    for(int i=1;i<8;i++){
    int socketfd=connectTo(ip);
    std::string request;
    request="<create><account id=\""+std::to_string(i+100)+"\" balance=\"1000000\"/><symbol sym=\"SPY\"><account id=\""+std::to_string(i+100)+"\">100000</account></symbol></create>";
    size_t size = request.size();
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, request.c_str(), request.size(), 0);
    
    size_t length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    std::vector<char> store=S.receiveall(socketfd,length);
    std::string respond(store.data());
    std::cout<<respond<<std::endl;
    close(socketfd);
    }
    //sleep(5);
    
    int socketfd=connectTo(ip);
    std::string req= "<transactions id=\"101\"><order sym=\"SPY\" amount=\"300\" limit=\"125\"/> </transactions>";
    size_t size = req.size();
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, req.c_str(), req.size(), 0);
  
    size_t length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    std::vector<char> store=S.receiveall(socketfd,length);
    std::string respond(store.data());
    std::cout<<respond<<std::endl;
    
    close(socketfd);
    
    socketfd=connectTo(ip);
    req= "<transactions id=\"102\"><order sym=\"SPY\" amount=\"-100\" limit=\"130\"/> </transactions>";
    size = req.size();
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, req.c_str(), req.size(), 0);
  
    length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    store=S.receiveall(socketfd,length);
    std::string respond1(store.data());
    std::cout<<respond1<<std::endl;
    
    close(socketfd);
    
    socketfd=connectTo(ip);
    req= "<transactions id=\"103\"><order sym=\"SPY\" amount=\"200\" limit=\"127\"/> </transactions>";
    size = req.size();
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, req.c_str(), req.size(), 0);
  
    length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    store=S.receiveall(socketfd,length);
    std::string respond2(store.data());
    std::cout<<respond2<<std::endl;
    
    close(socketfd);
    
    
    socketfd=connectTo(ip);
    req= "<transactions id=\"104\"><order sym=\"SPY\" amount=\"-500\" limit=\"128\"/> </transactions>";
    size = req.size();
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, req.c_str(), req.size(), 0);
  
    length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    store=S.receiveall(socketfd,length);
    std::string respond3(store.data());
    std::cout<<respond3<<std::endl;
    
    close(socketfd);
    
    socketfd=connectTo(ip);
    req= "<transactions id=\"105\"><order sym=\"SPY\" amount=\"-200\" limit=\"140\"/> </transactions>";
    size = req.size();
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, req.c_str(), req.size(), 0);
  
    length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    store=S.receiveall(socketfd,length);
    std::string respond4(store.data());
    std::cout<<respond4<<std::endl;
    
    close(socketfd);
    
    socketfd=connectTo(ip);
    req= "<transactions id=\"106\"><order sym=\"SPY\" amount=\"400\" limit=\"125\"/> </transactions>";
    size = req.size();
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, req.c_str(), req.size(), 0);
  
    length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    store=S.receiveall(socketfd,length);
    std::string respond5(store.data());
    std::cout<<respond5<<std::endl;
    
    close(socketfd);

    socketfd=connectTo(ip);
    req= "<transactions id=\"107\"><order sym=\"SPY\" amount=\"500\" limit=\"150\"/> </transactions>";//700 150
    size = req.size();
    send(socketfd, (char*)&size, sizeof(size_t),0);
    send(socketfd, req.c_str(), req.size(), 0);
  
    length= 0;
    recv(socketfd, (char *)&length, sizeof(size_t), MSG_WAITALL);
    store=S.receiveall(socketfd,length);
    std::string respond6(store.data());
    std::cout<<respond6<<std::endl;
    
    close(socketfd);

}

