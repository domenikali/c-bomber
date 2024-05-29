#ifndef VAR
#define VAR
#include <pthread.h>
struct Player
{
    int socket;
    int udpsocket;
    struct sockaddr_in6 udp_addr;
    uint16_t id;
    uint16_t eq;
};
typedef struct Player Player;

struct Partie {
    int index;
    Player *players;
    char* ip;
    uint16_t port_mdiff;
    uint16_t port_udp;
    int mode;
};

typedef struct Partie Partie;

struct Info_thread
{
    int sock;
    int udpsocket;
    Partie *partie;
};
typedef struct Info_thread Info_thread;
struct Tchat_Sender_Arg
{
    Partie *p;
    Info_thread *info;
    Message_TCP *msg;
};
typedef struct Tchat_Sender_Arg Tchat_Sender_Arg;




#endif