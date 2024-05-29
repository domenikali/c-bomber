#include "serveur_connexions.h"

int port_udp_solo=9091;
int port_udp_Duo=9090;
Partie Solo[MAX_PARTIE];
Partie Duo[MAX_PARTIE];
pthread_mutex_t mutexSolo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDuo = PTHREAD_MUTEX_INITIALIZER;
uint16_t new_movement[4];
pthread_mutex_t player_actions = PTHREAD_MUTEX_INITIALIZER;
int game_over;//0 if game is still going, 1 if the game is over
uint16_t packet_number = 0;
int MDIF_PORT = 9990;


int get_available_partie(Partie p[MAX_PARTIE]){
    for(int i = 0 ; i < MAX_PARTIE; i++){
        if(p[i].index < 4)
            return i;
    }
    return -1;
}
void destroy_partie(Partie* p ){
    p->index = 0;
    //TODO
}

uint16_t next_packet_number(){
    if(packet_number<6534){
        packet_number++;
        return packet_number;
    }
    else {
        packet_number = 0;
        return packet_number;
    }
}



void print_partie(Partie* partie){
    if (partie->mode==1)
    printf("Partie solo:\n");
    else printf("Partie duo:\n");
    
    print(LOG_INFO,"    Partie index:%d\n",partie->index);
    print(LOG_INFO,"    Partie ip:%s\n",partie->ip);
    print(LOG_INFO,"    Partie port_mdiff:%d\n",(int)partie->port_mdiff);
    print(LOG_INFO,"    Partie prt_udp:%d\n",(int)partie->port_udp);
    for (size_t i = 0; i < (size_t)(partie->index); i++)
    {
        print(LOG_INFO,"    id:%d ",partie->players[i].id);
        print(LOG_INFO,"eq:%d ",partie->players[i].eq);
        print(LOG_INFO,"socket:%d\n",partie->players[i].socket);
    }  
}

//create a tcp sockaddr struct open ready for the bind
struct sockaddr_in create_tcp_server_sockaddr(int port){
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    return server_address;
}

//create a udp ipv6 sockaddr struct opne on port ready for the bind
struct sockaddr_in6 create_udp_server_sockaddr(int port){
    struct sockaddr_in6  udp_server_addr;
    memset(&udp_server_addr, 0, sizeof(udp_server_addr));

    udp_server_addr.sin6_family = AF_INET6; // IPv6
    udp_server_addr.sin6_addr = in6addr_any;
    udp_server_addr.sin6_port = htons(port);
    return udp_server_addr;
}
// la fonction qui va envoyer le message a tout les autres joueurs ou a un seul joueur
void* tchat_sender(void* arg){
    Tchat_Sender_Arg *arg_tchat=(Tchat_Sender_Arg*)arg;
    Partie *p=arg_tchat->p;
    //Info_thread *info=arg_tchat->info;
    Message_TCP *msg=arg_tchat->msg;

    uint16_t codereq ;
    uint16_t id =0;
    uint16_t eq =0;
    decode_message(msg->header,&codereq,&id,&eq);
        if(DEBUG){
            printf("--------- Ligne 61 dans la fonction---------- \n");
            printf("TCP mes client header : CODEREQ = %d, ID = %d, EQ = %d\n", codereq, id, eq);
            printf("len: %d\n",msg->len);
            printf("message:\n");
            printf("data len est %ld et contenue :%s\n",strlen(msg->data),msg->data);
        }
    if (codereq==7)
    {
        
        msg->header=create_header(13,id,eq);
        if(DEBUG){
            printf("--------- Ligne 69 dans la fonction---------- \n");
            printf("TCP mes client header : CODEREQ = %d, ID = %d, EQ = %d\n", codereq, id, eq);
            printf("len: %d\n",msg->len);
            printf("message:\n");
            printf("data len est %ld et contenue :%s\n",strlen(msg->data),msg->data);
        }
        char message[MESSAGE_LEN];
        snprintf(message, MESSAGE_LEN, "Joueur num°%d: %s",id, msg->data);

        // Ensure the buffer is null-terminated
        message[MESSAGE_LEN - 1] = '\0';
        for (int k = 15+msg->len; k < MESSAGE_LEN; k++) {
            message[k] = ' ';
        }
        strncpy( msg->data, message, MESSAGE_LEN);
        for (size_t i = 0; i < 4; i++){
            int sockclient=p->players[i].socket;
            int bytes_sent;
            int total_sent = 0;
            while (total_sent < (int) sizeof(*msg)) {
            bytes_sent = send(sockclient, ((char *)msg) + total_sent, sizeof(*msg) - total_sent, 0);
            if (bytes_sent == -1) {
                perror("Send failed"); // Print error message if sending fails
                pthread_exit((void*)EXIT_FAILURE); // Exit if there is a send error
            }
            total_sent += bytes_sent; // Update total bytes sent
            }
        }
    }
    else{
        
        msg->header=create_header(14,id,eq);

        for (size_t i = 0; i < 4; i++){
            if (p->players[i].eq==eq&&p->players[i].id!=id)
            {
                int sockclient=p->players[i].socket;
                int bytes_sent;
                size_t total_sent = 0;
                while (total_sent < sizeof(*msg)) {
                    bytes_sent = send(sockclient, ((char *)msg) + total_sent, sizeof(*msg) - total_sent, 0);
                    if (bytes_sent == -1) {
                        perror("Send failed"); // Print error message if sending fails
                        pthread_exit((void*)EXIT_FAILURE); // Exit if there is a send error
                    }
                    total_sent += bytes_sent; // Update total bytes sent
                }
            }
        }
    }
    pthread_exit(0);
}
// la fonction qui va recevoir les messages et les envoyer a tout les autres joueurs en utilisant la fonction tchat_sender
void* tchat_recever(void*arg){
    Info_thread *info=(Info_thread*)arg;
    int sockclient=info->sock;
    Message_TCP tcp_mes;
    int bytes_received;
    int total_received;
    while (1)
    {
        bytes_received = 0;
        total_received = 0;
        while (total_received < (int)sizeof(tcp_mes)) {
            bytes_received = recv(sockclient, ((char *)&tcp_mes) + total_received, sizeof(tcp_mes) - total_received, 0);
            if (bytes_received == -1) {
                perror("Recv failed"); // Print error message if receiving fails
                pthread_exit((void*)EXIT_FAILURE); // Exit if there is a receive error
            }
            total_received += bytes_received; // Update total bytes received
        }
        Tchat_Sender_Arg arg;
        arg.p=info->partie;
        arg.info=info;
        arg.msg=&tcp_mes;
        // je cree un thread pour envoyer le message a tout les autres joueurs ou au binome
        pthread_t thread_tchat_sender;
        pthread_create(&thread_tchat_sender,NULL,(void*)tchat_sender,&arg);
    }
}

void action_recever(void*arg){
    Info_thread *info=(Info_thread*)arg;
    int udp_socket=info->udpsocket;
    Message_UDP_action udp_message;
    if (recvfrom(udp_socket, &udp_message, sizeof(udp_message), 0, NULL, NULL) != sizeof(udp_message)) {
        perror("recvfrom UDP message");
        exit(1);
    }

    uint16_t codereq_udp;
    uint16_t id_udp=0;
    uint16_t eq_udp=0;
    decode_message(udp_message.header, &codereq_udp, &id_udp, &eq_udp);

    if(DEBUG) {
        printf("Header du client Action normalement : CODEREQ = %d, ID = %d, EQ = %d\n", codereq_udp, id_udp, eq_udp);
    }
}
// la fonction qui va initialiser la partie en donnant les valeurs par defaut et les port et addresse
void int_Partie(Partie *p, int mode) {
    p->ip = new_mdiff_adress();
    p->port_mdiff = MDIF_PORT++;
     if (mode ==2)
        {
            p->port_udp = new_udp_port(port_udp_Duo);
            port_udp_Duo++;
        }else{
            p->port_udp = new_udp_port(port_udp_solo);
            port_udp_solo++;
        }
    
    p->mode = mode;
    p->index=0;
    p->players = calloc(4, sizeof(Player));
    if (p->players == NULL) {
        fprintf(stderr, "Allocation de mémoire échouée.\n");
        pthread_exit((void*)EXIT_FAILURE);
    }

    for (int i = 0; i < 4; i++) {
        p->players[i].socket = -1; // Socket non assigné initialement
        p->players[i].id = i;
        if (mode ==2)
        {
            p->players[i].eq = (uint16_t) (i % 2);
        }else{
            p->players[i].eq = 0;
        }
    }
}

//return a sockaddr_in struct after accepting a client on sock and opening a client sock 
struct sockaddr_in accept_client(int sock, int * sock_client){
    
    struct sockaddr_in client_address;
    memset(&client_address, 0, sizeof(client_address));
    socklen_t client_address_len = sizeof(client_address);
    *sock_client = accept(sock, (struct sockaddr *)&client_address, &client_address_len);
    if (*sock_client == -1) {
        perror("probleme socket client");
        exit(1);
    }
    return client_address;
} 

/**
*   Recive udp packets from the clients on the socket,
*   decode the header and asigne a new requested position to the player who sent the packet,
*   then continu to loop
*   @return 0 at the end of the game
*   @param UDP socket casted as (void*)
*   @author Leo
*/
void * udp_reciver(void * arg){
    int udp_sock = *(int*)arg;
    Message_UDP_action action;
    uint16_t codereq;
    uint16_t id;
    uint16_t eq;

    
    while(!game_over){
        //print(LOG_INFO,"wating for packet\n");
        if(recv(udp_sock,&action,sizeof(action),0)!=sizeof(action)){
            perror("send err");
            pthread_exit((void*)EXIT_FAILURE);
        }
        decode_message(action.header,&codereq,&id,&eq);
        pthread_mutex_lock(&player_actions);//the mutex here slow down the logic
        new_movement[id]=action.action;
        pthread_mutex_unlock(&player_actions);
        //print(LOG_INFO,"packet recived\n");

    }
    return (void*)0;


}

/**
*   create a mdiff structure by the parameter passed, serialize the grid
*   @return Mdiff_stat structure
*   @param header : message_header 
*   @param num : the number of the packet
*   @param grid: the game grid structure
*   @author Leo 
*/
Mdiff_stat create_mdiff_struct(Message_header header,uint16_t num,Grid grid){
    Mdiff_stat packet;
    packet.header = header;
    packet.num =num;
    packet.hauteur = grid.height;
    packet.largeur = grid.width;
    packet.cases = my_malloc(sizeof(uint8_t)*grid.height*grid.width);

    int index=0;
    for(int i =0;i<grid.height;i++){
        for(int j=0;j<grid.width;j++){
            packet.cases[index++]=grid.map[i][j];
        }
    }

    return packet;
}
/**
*   send the full grid on multidiffusion
*   @param mdiff_server_socket : multidiffusion scoket
*   @param gradr : struct with port,adress and group information
*   @param mdiff_struct : structure to send
*   @author Leo
*/
void mdiff_full_grid(int mdiff_server_sock ,struct sockaddr_in6 gradr ,Mdiff_stat mdiff_struct){
    size_t packet_size;
    char * packet = serialize_Mdiff(mdiff_struct,&packet_size);
    print(LOG_INFO,"grid serialized\n");
    if(sendto(mdiff_server_sock,packet,packet_size,0,(struct sockaddr*)&gradr, sizeof(gradr))!=(int)packet_size){
        perror("send err");
        exit(1);
        close(mdiff_server_sock);
    }
    char c[INET_ADDRSTRLEN];
    inet_ntop(AF_INET6,&gradr.sin6_addr,c,sizeof(c));
    print(LOG_INFO,"grid serialized port gradr %d\n",ntohs(gradr.sin6_port));
    print(LOG_INFO,"grid serialized adresse gradr %c\n",c);

    print(LOG_INFO,"Full grid sent\n");
    free(packet);
}
/**
*   send the partial grid on multidiffusion
*   @param mdiff_server_socket : multidiffusion scoket
*   @param gradr : struct with port,adress and group information
*   @param partial : structure to send with partial update
*   @author Leo
*/
void mdiff_partial_grid(int mdiff_server_sock ,struct sockaddr_in6 gradr ,Partial_grid partial){
    size_t packet_size;
    char * packet = serialize_partial(partial,&packet_size);
    print(LOG_INFO,"grid serialized\n");
    if(sendto(mdiff_server_sock,packet,packet_size,0,(struct sockaddr*)&gradr, sizeof(gradr))!=(int)packet_size){
        perror("send err");
        exit(1);
        close(mdiff_server_sock);
    }
    print(LOG_INFO,"Partial grid sent\n");
    free(packet);
}


void * integre_player(void *arg){
    int sockclient=*((int *)arg);
    // Réception de la demande d'integration du client
    Message_header mes;
    int bytes_received;
    size_t total_received = 0;
    while (total_received < sizeof(mes)) {
        bytes_received = recv(sockclient, ((char *)&mes) + total_received, sizeof(mes) - total_received, 0);
        if (bytes_received == -1) {
            perror("Recv failed"); // Print error message if receiving fails
            pthread_exit((void*)EXIT_FAILURE); // Exit if there is a receive error
        }
        total_received += bytes_received; // Update total bytes received
    }

      // Conversion du message en format host
    uint16_t codereq ;
    uint16_t id =0;
    uint16_t eq =0;

    decode_message(mes,&codereq,&id,&eq);

    // creation de la partie ou remplissage de la partie ca dependra du remplissage du tableau et du mode 
    Partie* partie;
    if (codereq==1)
    {
        //add mutexS
        int available_partie = get_available_partie(Solo);
        if(available_partie == -1)
            //send error packet to player (NO PARIE AVAIALABLE)
        
        pthread_mutex_lock(&mutexSolo);
        if (Solo[available_partie].index>=4||Solo[available_partie].index==0)
        {   
            int_Partie(&Solo[available_partie],1);
        }
        partie=&Solo[available_partie];
        partie->players[partie->index].socket=sockclient;
        id=(partie->index)++;
        eq=partie->players[id].eq;
        pthread_mutex_unlock(&mutexSolo);
    }else{
        int available_partie = get_available_partie(Duo);
        if(available_partie == -1)
            //send error packet to player (NO PARIE AVAIALABLE)
        //add mutex
        pthread_mutex_lock(&mutexDuo);
        if (Duo[available_partie].index>=4||Duo[available_partie].index==0)
        {   
            int_Partie(&Duo[available_partie],2);
        }
        partie=&Duo[available_partie];
        partie->players[partie->index].socket=sockclient;
        id=partie->index++;
        eq=partie->players[id].eq;
        pthread_mutex_unlock(&mutexDuo);
    }
    
    //create_header changer codere
    Message_header header = create_header(codereq==1?9:10,id,eq);
    char *mdiff_ip=partie->ip;
    Integr_partie adress = join_new_game(header,partie->port_udp,partie->port_mdiff,mdiff_ip);

    //send Integer_partie packet
    int bytes_sent;
    size_t total_sent = 0;
    while (total_sent < sizeof(adress)) {
        bytes_sent = send(sockclient, ((char *)&adress) + total_sent, sizeof(adress) - total_sent, 0);
        if (bytes_sent == -1) {
            perror("Send failed"); // Print error message if sending fails
            pthread_exit((void*)EXIT_FAILURE); // Exit if there is a send error
        }
        total_sent += bytes_sent; // Update total bytes sent
    }
    Message_header pret;
    total_received = 0;
    while (total_received < sizeof(pret)) {
        bytes_received = recv(sockclient, ((char *)&pret) + total_received, sizeof(pret) - total_received, 0);
        if (bytes_received == -1) {
            perror("Recv failed"); // Print error message if receiving fails
            pthread_exit((void*)EXIT_FAILURE); // Exit if there is a receive error
        }
        total_received += bytes_received; // Update total bytes received
    }

    decode_message(pret,&codereq,&id,&eq);
    
    if(DEBUG){
        printf("Header du client pret : CODEREQ = %d, ID = %d, EQ = %d\n", codereq, id, eq);

        if(codereq==3){
            debug_print("Pret mode Solo");
        }if(codereq==4){
            debug_print("Pret mode Duo");
        }
    }

    // Soit on arret ici on ferme le thread car on a fini le traitement du client sinon si le tableau est remplit on lance la partie
    if (partie->index==4)
    {
        //lancer la partie
        printf("\n ---------------------Lancer la partie---------------------\n");
        print_partie(partie);
        printf("------------------------------------------------------------\n");
        pthread_t thread_tchats[4];
        Info_thread info_thread[4];
        game_over=0;
        for (size_t i = 0; i < 4; i++)
        {
            info_thread[i].sock=partie->players[i].socket;
            info_thread[i].partie=partie;
            pthread_create(&thread_tchats[i],NULL,(void*)tchat_recever,&(info_thread[i]));
        }

        //lancer_partie(partie);
        //udp socket creation
        int *udp_server_sock = malloc(sizeof(int));
        struct sockaddr_in6  udp_server_addr = create_udp_server_sockaddr(partie->port_udp);
        if ((*udp_server_sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
            perror("Socket creation failed");
            pthread_exit((void*)EXIT_FAILURE);
        }   
        // Bind the socket with the server address
        if (bind(*udp_server_sock, (const struct sockaddr *)&udp_server_addr, sizeof(udp_server_addr)) < 0) {
            perror("Bind failed");
            pthread_exit((void*)EXIT_FAILURE);
        }
        //thread creation for the UDP reciver
        pthread_t thread_udp_reciver;
        pthread_create(&thread_udp_reciver,NULL,udp_reciver,udp_server_sock);

        

        //mdiff socket creation
        int mdiff_server_sock;
        mdiff_server_sock = socket(AF_INET6, SOCK_DGRAM, 0);
        
        
        struct sockaddr_in6 gradr;
        memset(&gradr, 0, sizeof(gradr));
        gradr.sin6_family = AF_INET6;
        inet_pton(AF_INET6, partie->ip, &gradr.sin6_addr);
        gradr.sin6_port = htons(partie->port_mdiff);

        print(LOG_INFO,"mdiff created\tip:%s port:%d\n",partie->ip,partie->port_mdiff);

        int ifindex = if_nametoindex("eth0");
        if(ifindex == 0)
            perror("if_nametoindex");

        gradr.sin6_scope_id = ifindex;

        //client side the game should start only afeter reciving the map
        //grid creation
        uint8_t width=23,height=23;
        Grid grid = create_new_map(width,height);

        //creation of first mdiff packet
        Message_header full_grid_header = create_header(11,0,0);// this header will be utlized
        uint16_t packet_num = next_packet_number(); //packet number will be reused
        Mdiff_stat mdiff_struct = create_mdiff_struct(full_grid_header,packet_num,grid);
        print(LOG_INFO,"struct crated\n");

        //send grid throug through mdiff packet

        mdiff_full_grid(mdiff_server_sock , gradr , mdiff_struct);

        //game logic
        game_over=0;
        //server game loop
        Bombs bombs =bombs_init(grid);
        sleep(1);
        print_grid(grid);
        Partial_grid * partial = my_malloc(sizeof(Partial_grid));
        memset(partial,0,sizeof(*partial));
        partial->header = create_header(12,0,0);
        while(!game_over){
            
            packet_num = next_packet_number();
            partial->nb=0;

            pthread_mutex_lock(&player_actions);
            for(int i=0;i<4;i++){
                if(new_movement[i]!=0){
                    uint16_t num,act;
                    decode_action(new_movement[i],&num,&act);
                    print(LOG_INFO,"player %d: num:%ld act:%ld\n",i,num,act);
                    move_player(&grid, i, act, &bombs, partial);
                    new_movement[i]=0;
                }
            }
            pthread_mutex_unlock(&player_actions);
            bomb_update(&bombs , &grid, partial);
            print_grid(grid);
            

            
            //one in 10 packets its the full map
            //commented to not saturate client in tests ,but it's working ;)
            if(partial->num % 10 != 0){
                partial->num = packet_num;
                print(LOG_INFO,"mdiff_partial_grid");
                mdiff_partial_grid(mdiff_server_sock,gradr,*partial);
            }
            else{
                mdiff_struct = create_mdiff_struct(full_grid_header,packet_num,grid);
                print(LOG_INFO,"mdiff_full_grid");
                mdiff_full_grid(mdiff_server_sock,gradr,mdiff_struct);
            }

            sleep(0.1);
        }
        free(partial->cases);
        free(partial);     

    }else{
        // arreter le thread
        pthread_exit(0);
    }
    
    return NULL;

}