#include "client_connexions.h"
//return a tcp sockaddr_in struct with ipv4 and port passed by argument
struct sockaddr_in create_tcp_client_sockaddr (Server_info server_info){
    struct sockaddr_in address_sock;
    memset(&address_sock, 0, sizeof(address_sock));
    address_sock.sin_family = AF_INET;
    address_sock.sin_port = htons(server_info.server_port);
    inet_pton(AF_INET, server_info.server_ip_adress, &address_sock.sin_addr);

    return address_sock;
}

//return a udp sockaddr_in6 with ipv6 and port passed by argument
struct sockaddr_in6 create_udp_client_sockaddr(char* ipv6,int port){
    struct sockaddr_in6 udp_server_addr;
    memset(&udp_server_addr, 0, sizeof(udp_server_addr));
    udp_server_addr.sin6_family = AF_INET6; // IPv6
    inet_pton(AF_INET6, ipv6, &udp_server_addr.sin6_addr);
    udp_server_addr.sin6_port = htons(port);
    return udp_server_addr;
}

//create a Message_TCP struct and send it to the socket passed
int send_tcp_chat_msg(int sock,Message_header header,char*msg,int msg_len){
    Message_TCP chat_msg;

    chat_msg.header = header;
    chat_msg.len = msg_len;
    memset(chat_msg.data,0,sizeof(chat_msg.data));

    strcpy(chat_msg.data,msg);
    //printf("Dans send_tcp_chat_msg la taille est %d et la chaine : %s\n",chat_msg.len,chat_msg.data);
    int bytes_sent;
    size_t total_sent = 0;
    while (total_sent < sizeof(chat_msg)) {
        bytes_sent = send(sock, ((char *)&chat_msg) + total_sent, sizeof(chat_msg) - total_sent, 0);
        if (bytes_sent == -1) {
            perror("Send failed"); // Print error message if sending fails
            exit(1); // Exit if there is a send error
        }
        total_sent += bytes_sent; // Update total bytes sent
    }
    return 0;
}