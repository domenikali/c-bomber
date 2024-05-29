#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <endian.h>
#include <stdarg.h> 
#include <pthread.h>

#define DEBUG 1
#define MESSAGE_LEN 1024
#define UDP_SRTART_POINT 9000
#define IPV6_ADRESS_START "ff12:f140:1f10:"

// ANSI color escape codes
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"

typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

typedef uint16_t Message_header;
typedef uint16_t Message_action;
typedef Message_header Debut_parite;
typedef Message_header Fin_partie;

struct Message_UDP_action{
    Message_header header;
    Message_action action;
};
typedef struct Message_UDP_action Message_UDP_action;

typedef struct{
    Message_header header;
    uint8_t len;
    char data[MESSAGE_LEN];
} Message_TCP;

//tout en big-endian
struct Integr_partie{
    Message_header header;
    uint16_t port_udp;
    uint16_t port_mdiff;
    uint8_t ip_mdiff[16];
};
typedef struct Integr_partie Integr_partie;


//header en big-endiand et aussi num;
struct Mdiff_stat{
    Message_header header;
    uint16_t num;
    uint8_t hauteur;
    uint8_t largeur;
    uint8_t * cases;    // len of cases = hauteur * largeur
};
typedef struct Mdiff_stat Mdiff_stat;

struct Case{
    uint8_t ligne;
    uint8_t colonne;
    uint8_t contenu;       
};
typedef struct Case Case;

struct Partial_grid{
    Message_header header;
    uint16_t num;
    uint8_t nb;
    Case * cases;
};
typedef struct Partial_grid Partial_grid;

Message_header create_header(uint16_t codereq, uint16_t id, uint16_t eq);

Message_action create_action(uint16_t num_action,uint16_t action);

int decode_action(Message_action action,uint16_t* num, uint16_t* act );


int decode_message(uint16_t message, uint16_t *codereq, uint16_t *id, uint16_t *eq);

Message_TCP create_tcp_message(Message_header header, char * mes);

Message_UDP_action create_udp_message(Message_header header, Message_action action);

uint8_t tstrtol_8(char *snb);

int chtoint8(char * ipv6,uint8_t*ip_mdiff);

char * int8toch(uint8_t * ipv6);

Integr_partie join_new_game(Message_header header,uint16_t port_udp,uint16_t port_mdiff,char * ipv6);

void debug_print(char * mes);

char * new_mdiff_adress();

int new_udp_port(int port);

void* my_malloc(size_t bytes);

/**
 * @brief : This function deserialize MDIFF packet to a buffer.
 * @param char_msg: The buffer to deserialize.
 * @return : Mdiff_stat structure.
*/
Mdiff_stat deserialize_Mdiff(char* char_msg);

char* serialize_Mdiff(Mdiff_stat stat,size_t* len);

char* serialize_partial(Partial_grid partial,size_t* len);

Partial_grid deserialize_partial(char* char_msg);


void  print(LogLevel level, const char *format, ...) ;
#endif