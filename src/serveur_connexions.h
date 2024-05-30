#ifndef SERVEUR_CONNEXIONS_H
#define SERVEUR_CONNEXIONS_H

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "var_glob.h"
#include <net/if.h>
#include "game.h"


struct Player_new_action{
    uint16_t player0_action;
    uint16_t player1_action;
    uint16_t player2_action;
    uint16_t player3_action;
};
typedef struct Player_new_action Player_new_action;


//temporary defined port
//#define UDP_PORT 8880
#define MAX_PARTIE 10



uint16_t next_packet_number();
void print_partie(Partie* partie);
struct sockaddr_in create_tcp_server_sockaddr(Server_info Server_info);
struct sockaddr_in6 create_udp_server_sockaddr(int port);
struct sockaddr_in accept_client(int sock, int * sock_client);
Mdiff_stat create_mdiff_struct(Message_header header,uint16_t num,Grid grid);
void mdiff_full_grid(int mdiff_server_sock ,struct sockaddr_in6 gradr ,Mdiff_stat mdiff_struct);
void mdiff_partial_grid(int mdiff_server_sock ,struct sockaddr_in6 gradr ,Partial_grid partial);

void * integre_player(void *arg);
void int_Partie(Partie *p, int mode) ;
void* tchat_sender(void* arg);
void* tchat_recever(void*arg);




#endif  