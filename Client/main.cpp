#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

char message[1024];
char buf[sizeof(message)];

void useUDP(){

    int sock_out = socket(AF_INET, SOCK_DGRAM, 0);
    int sock_in = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_out < 0 || sock_in < 0) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in addr_out;
    addr_out.sin_family = AF_INET;
    addr_out.sin_port = htons(23445);
    addr_out.sin_addr.s_addr = htonl(INADDR_ANY);
    struct sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(23447);
    addr_in.sin_addr.s_addr = htonl(INADDR_ANY);   
    if(bind(sock_in, (struct sockaddr *)&addr_in, sizeof(addr_in)) < 0) {
        perror("bind");
        exit(2);
    } 
    
    std::cout << "To complete, type \"*\"" << std::endl;
    
    while(true) {
        std::cin >> message;
        if(message[0] == '*') break;
        
        sendto(sock_out, message, sizeof(message), 0, (struct sockaddr *)&addr_out, sizeof(addr_out));
        recvfrom(sock_in, buf, sizeof(message), 0, NULL, NULL);
        
        std::cout << buf << std::endl;
    }    

    close(sock_out);
    close(sock_in);
}

void useTCP() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(23444);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(2);
    }
    
    std::cout << "To complete, type \"*\"" << std::endl;
    
    while(true) {
        std::cin >> message;
        if(message[0] == '*') break;
        
        send(sock, message, sizeof(message), 0);
        recv(sock, buf, sizeof(message), 0);
        
        std::cout << buf << std::endl;
    }
    
    close(sock);    
}

int main()
{
    std::cout << "UDP: 0" << std::endl << "TCP: 1" << std::endl;
    int n;
    std::cin >> n;
    
    bool ka = true;

    while(ka){
        switch(n){
            case 0:
                useUDP();
                ka = false;
                break;
            case 1:
                useTCP();
                ka = false;
                break;
            default:
                std::cout << "Wrong input. Select again." << std::endl;
        }
    }
    

    return 0;
}