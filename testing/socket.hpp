#include <iostream>
#include <stdio.h>
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
#include<vector>
#define PORT "12345"

#ifndef socket_hpp
#define socket_hpp

#include <stdio.h>

#endif /* socket_hpp */

class Socket{
public:
    int BuildSocket();
    int ConnectTo(std::string &ip);
    std::vector<char> receiveall(int socketfd){
        std::vector<char> store;
        while(true){
            std::vector<char> temp(1,0);
            int size = recv(socketfd, temp.data(), temp.size(), 0);
            if(size==0){
                break;
            }
            for(int i=0;i<size;i++){
                store.push_back(temp[i]);
            }
            
            if(store.size()==0){
                store.push_back('\0');
                break;
            }
        }
        return store;
    }
};
