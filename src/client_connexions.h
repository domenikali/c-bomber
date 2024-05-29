#ifndef CLIENT_CONNEXIONS_H
#define CLIENT_CONNEXIONS_H

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include "utils.h"

struct sockaddr_in create_tcp_client_sockaddr (char* ipv4_addr,int port);
struct sockaddr_in6 create_udp_client_sockaddr(char* ipv6,int port);
int send_tcp_chat_msg(int sock,Message_header header,char*msg,int msg_len);


#endif