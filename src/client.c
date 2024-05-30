#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include "utils.h"
#include <net/if.h>
#include <pthread.h>
#include "var_glob.h"
#include "client_connexions.h"
#include "ncurs.h"
#include <ncurses.h> // ncurses pour l'input du clavier

#define ADDR "127.0.0.1" // Adresse IP du serveur
pthread_mutex_t verrouTchat = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t verrouGrid = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t verrouLine = PTHREAD_MUTEX_INITIALIZER;
board* b;
line* l;
int nbchat;
char chathistory[MAX_CHAT_MESAGES][TEXT_SIZE];
void miseAjourBoard(board* b, Mdiff_stat header) {
    b->w = header.largeur;
    b->h = header.hauteur;

    memcpy(b->grid, header.cases, b->w * b->h * sizeof(uint8_t));
}
void miseAjourPartielBoard(board* b, Partial_grid pg) {
    for (int i = 0; i < pg.nb; i++) {
        Case caseCourante = pg.cases[i];
        if (caseCourante.ligne < b->h && caseCourante.colonne < b->w) {
            b->grid[caseCourante.ligne * b->w + caseCourante.colonne] = caseCourante.contenu;
        } else {
            fprintf(stderr, " Erreur : case (%d,  %d) hors  de la grille \n", caseCourante.ligne, caseCourante.colonne);
        }
    }
}
void tchat_sender(line*l,Player *player){
    int fdsock = player->socket;
        Message_header header;
        if(l->data[0]=='A')  header = create_header(7,player->id,player->eq);
        else header= create_header(8,player->id,player->eq);
        send_tcp_chat_msg(fdsock,header,l->data,l->cursor);
        memset(l->data,0,sizeof(l->data));
        l->cursor=0;
}


Message_TCP tchat_recevercopy(int fdsock){

    Message_TCP chat_msg;
        int bytes_received;
        size_t total_received = 0;
        while (total_received < sizeof(chat_msg)) {
            bytes_received = recv(fdsock, ((char *)&chat_msg) + total_received, sizeof(chat_msg) - total_received, 0);
            if (bytes_received == -1) {
                perror("Recv failed");
                exit(1);
            }
            total_received += bytes_received;
        }

        uint16_t id =0;
        uint16_t eq =0;
        uint16_t codereq=0;
        return chat_msg;
}




void* tchat_recever(void*arg){
    int fdsock =*(int*)arg;

    while (1)
    {
        Message_TCP chat_msg;
        int bytes_received;
        size_t total_received = 0;
        while (total_received < sizeof(chat_msg)) {
            bytes_received = recv(fdsock, ((char *)&chat_msg) + total_received, sizeof(chat_msg) - total_received, 0);
            if (bytes_received == -1) {
                perror("Recv failed");
                exit(1);
            }
            total_received += bytes_received;
        }

        uint16_t id =0;
        uint16_t eq =0;
        uint16_t codereq=0;
        decode_message(chat_msg.header,&codereq,&id,&eq);
        if (codereq==13)
        {
            printf("Message reçu du serveur envoyer par le joueur numero %d à tout le monde : %s\n",id,chat_msg.data);
        }else
            printf("Message reçu du serveur envoyer par le joueur numero %d: %s\n",id,chat_msg.data);
    }
}
void actionSender(int udp_client_sock,struct sockaddr_in6 udp_server_addr, int direction,int numaction,int codereq,Player player){
    if (direction != -1) {
            Message_header header = create_header(codereq, player.id,player.eq);
            Message_action action;
            action = create_action(numaction, direction);
            Message_UDP_action message = create_udp_message(header, action);
            sendto(udp_client_sock, &message, sizeof(message), 0, (const struct sockaddr *)&udp_server_addr, sizeof(udp_server_addr));
            direction = -1; // Réinitialiser la direction après l'envoi
            if(numaction==8190){
                numaction=1;
            }
            else{
                numaction++;
            }
        }
}

int main(int argc, char *argv[]) {

    // Création du socket
    int fdsock = socket(PF_INET, SOCK_STREAM, 0);
    if (fdsock == -1) {
        perror("creation socket");
        exit(1);
    }

    // Création de l'adresse du serveur
    Server_info Server_info = get_server_info();
    struct sockaddr_in address_sock = create_tcp_client_sockaddr(Server_info);


    // Tentative de connexion au serveur
    int r = connect(fdsock, (struct sockaddr *)&address_sock, sizeof(address_sock));
    if (r == -1) {
        perror("echec de la connexion");
        close(fdsock);
        exit(2);
    }

    debug_print("Connexion établie avec le serveur");
    uint16_t codereq=0;
    while(codereq!=1&&codereq!=2){
        printf("Quelle mode voulez-vous integrer ?\n\t1: mode 4 adversaires\n\t2: mode équipes\n");
        scanf("%hd", &codereq);
    }

    //send a message asking to be put in queue on the desired mod
    Debut_parite demarre_connexion = create_header(codereq,0,0);

    int bytes_sent;
    size_t total_sent = 0;
    while (total_sent < sizeof(demarre_connexion)) {
        bytes_sent = send(fdsock, ((char *)&demarre_connexion) + total_sent, sizeof(demarre_connexion) - total_sent, 0);
        if (bytes_sent == -1) {
            perror("Send failed"); // Print error message if sending fails
            exit(1); // Exit if there is a send error
        }
        total_sent += bytes_sent; // Update total bytes sent
    }


    print(LOG_INFO,"Queue message sent\n");

    //recive the game info
    Integr_partie adress;
    int bytes_received;
    size_t total_received = 0;
    while (total_received < sizeof(adress)) {
        bytes_received = recv(fdsock, ((char *)&adress) + total_received, sizeof(adress) - total_received, 0);
        if (bytes_received == -1) {
            perror("Recv failed");
            exit(1);
        }
        total_received += bytes_received;
    }

    //recive id and eq
    uint16_t id =0;
    uint16_t eq =0;

    decode_message(adress.header,&codereq,&id,&eq);
    uint16_t port_udp = ntohs(adress.port_udp);
    uint16_t port_mdiff = ntohs(adress.port_mdiff);
    char * ip = int8toch(adress.ip_mdiff);
    Player player = {.eq=eq, .id=id, .socket=fdsock, .udpsocket=0, .udp_addr={0}};

    print(LOG_INFO,"header: codereq: %d,id: %d, eq:%d\n",codereq,id,eq);
    print(LOG_INFO,"port udp: %d,port mdiff:%d\n",port_udp,port_mdiff);
    print(LOG_INFO,"ip mdif:%s\n",ip);

    //send ready confirmation
    Message_header pret = create_header(codereq==9?3:4,id,eq);
    total_sent = 0;
    while (total_sent < sizeof(pret)) {
        bytes_sent = send(fdsock, ((char *)&pret) + total_sent, sizeof(pret) - total_sent, 0);
        if (bytes_sent == -1) {
            perror("Send failed");
            exit(1);
        }
        total_sent += bytes_sent; // Update total bytes sent
    }


    //udp responce
    int udp_client_sock;

    if ((udp_client_sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in6 udp_server_addr = create_udp_client_sockaddr("::1",port_udp);


    player.udpsocket=udp_client_sock;
    player.udp_addr=udp_server_addr;

    //inscription to mdiff and wait for grid to start
    int mdiff_client_sock;
    if((mdiff_client_sock = socket(AF_INET6, SOCK_DGRAM, 0))<0){
        close(mdiff_client_sock);
        exit(1);
    }


    //inscription sock option for reuse
    int ok = 1;
    if(setsockopt(mdiff_client_sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
        perror("sockoption err:");
        close(mdiff_client_sock);
        return 1;
    }

    //recive inscription sock struct
    struct sockaddr_in6 mdif_sockaddr;
    memset(&mdif_sockaddr, 0, sizeof(mdif_sockaddr));
    mdif_sockaddr.sin6_family = AF_INET6;
    mdif_sockaddr.sin6_addr = in6addr_any;
    mdif_sockaddr.sin6_port = htons(port_mdiff);//port sent by server

    //bind to multicast
    if(bind(mdiff_client_sock, (struct sockaddr*) &mdif_sockaddr, sizeof(mdif_sockaddr))) {
        perror("echec de bind");
        close(mdiff_client_sock);
        return 1;
    }

    //multicast protocol
    int ifindex = if_nametoindex ("enp1s0");
    if(ifindex == 0)
        perror("prototocl err:");

    // insciption to multicast
    struct ipv6_mreq group;
    inet_pton (AF_INET6, ip, &group.ipv6mr_multiaddr.s6_addr);
    group.ipv6mr_interface = ifindex;
    if(setsockopt(mdiff_client_sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof group) < 0) {
        perror("inscription err:");
        close(mdiff_client_sock);
        return 1;
    }

    //when the grid is recived the game can start
    char buffer[1024];
    memset(buffer,0,strlen(buffer));
    print(LOG_INFO,"buffer created\n");
    recvfrom(mdiff_client_sock, buffer, sizeof(buffer), 0, NULL, NULL);
    print(LOG_INFO,"message recived\n");
    Mdiff_stat stat = deserialize_Mdiff(buffer);
    print(LOG_INFO,"num:%ld, height:%d, width:%d\n",stat.num,stat.hauteur,stat.largeur);



    // Traitement de l'entrée clavier
    int numaction=1;

    b = malloc(sizeof(board));
    l = malloc(sizeof(line));
    l->data[TEXT_SIZE-1]=0;
    l->cursor = 0;
    pos* p = malloc(sizeof(pos));
    p->x = 0; p->y = 0;
    nbchat=0;
    // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done on the same thread.
    initscr(); /* Start curses mode */
    raw(); /* Disable line buffering */
    intrflush(stdscr, FALSE); /* No need to flush when intr key is pressed */
    keypad(stdscr, TRUE); /* Required in order to get events from keyboard */
    nodelay(stdscr, TRUE); /* Make getch non-blocking */
    noecho(); /* Don't echo() while we do getch (we will manually print characters when relevant) */
    curs_set(0); // Set the cursor to invisible
    start_color(); // Enable colors
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Define a new color style (text is yellow, background is black)

    setup_board(b);


    int listener = mdiff_client_sock;
    int fd_size = 3;
    struct pollfd *pfds = malloc(sizeof(*pfds) * fd_size);
    memset(pfds, 0, sizeof(*pfds) * fd_size);
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    pfds[1].fd = listener;
    pfds[1].events = POLLIN;
    pfds[2].fd= fdsock;
    pfds[2].events= POLLIN;

    int fd_count=3;
    refresh_game(b,l,chathistory,nbchat);
    while(1){
        int poll_cpt = poll(pfds, fd_count, 10000);
        if(poll_cpt == 0) {
        printf("je m'ennuie...\n");
        continue;
        }
        for(int i = 0; i < fd_count; i++) {
            if (pfds[i].revents & POLLIN) {
                if (pfds[i].fd == STDIN_FILENO) {
                    ACTION a = control(l);
                    int action=-1;
                    switch (a) {
                        case LEFT:
                            action = 2; break;
                        case RIGHT:
                            action = 1; break;
                        case UP:
                            action = 0; break;
                        case DOWN:

                            action = 3; break;
                        case QUIT:
                            goto end;
                            action=-2; break;
                        case BOMB:
                            action = 4;
                            break;
                        case SEND:

                            action= 5;
                        default:
                            break;
                    }

                    if (action >=0 && action <5)
                    {
                        actionSender(udp_client_sock,udp_server_addr,action,numaction,codereq,player);
                    }else if(action ==5){
                        tchat_sender(l,&player);
                        memset(l->data,0,sizeof(l->data));
                    }
                    refresh_game(b,l,chathistory,nbchat);
                }else if(pfds[i].fd == listener){
                    size_t message_size=2048;
                    char *message= my_malloc(message_size);
                    recvfrom(mdiff_client_sock,message, message_size, 0, NULL, NULL);
                    Message_header *header=my_malloc(sizeof(Message_header));
                    memcpy(header, message, sizeof(Message_header));
                    uint16_t reqm;
                    uint16_t idm;
                    uint16_t eqm;
                    decode_message(*header,&reqm,&idm,&eqm);
                    free(header);
                    if (reqm==11)
                    {
                        Mdiff_stat magrille= deserialize_Mdiff(message);
                        free(message);
                        miseAjourBoard(b,magrille);
                        refresh_game(b,l,chathistory,nbchat);
                    }else{
                        Partial_grid magrillePartielle= deserialize_partial(message);
                        free(message);
                        miseAjourPartielBoard(b,magrillePartielle);
                        refresh_game(b,l,chathistory,nbchat);
                    }
                }else{
                    Message_TCP messagetcp= tchat_recevercopy(fdsock);
                    memcpy(chathistory[nbchat++],messagetcp.data,messagetcp.len);
                    refresh_game(b,l,chathistory,nbchat);
                }

            }
        }
    }
end:

    free_board(b);

    curs_set(1); // Set the cursor to visible again
    endwin(); /* End curses mode */

    free(p); free(l); free(b);


    Message_header test_mdif;
    recvfrom(mdiff_client_sock, &test_mdif, sizeof(test_mdif), 0, NULL, NULL);

    decode_message(test_mdif,&codereq,&id,&eq);
    if(DEBUG){
        printf("mdif mes client header : CODEREQ = %d, ID = %d, EQ = %d\n", codereq, id, eq);

    }

    //close sockets
    endwin();
    close(udp_client_sock);
    free(ip);
    close(fdsock);

    return 0;
}