
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "utils.h"
#include "serveur_connexions.h"
#include <net/if.h>
#include <pthread.h>
#include "game.h"

#define BUFFER_SIZE 255

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print(LOG_ERROR,"Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);
    

    // Création du socket
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("creation socket");
        exit(1);
    }

    // Configuration de l'adresse du serveur
    struct sockaddr_in server_address = create_tcp_server_sockaddr(port);
    

    // Liaison du socket à l'adresse et au port
    int r = bind(sock, (struct sockaddr *)&server_address, sizeof(server_address));
    if (r < 0) {
        perror("erreur bind");
        exit(2);
    }

    // Écoute des connexions entrantes
    r = listen(sock, 0);
    if (r < 0) {
        perror("erreur listen");
        exit(2);
    }

    print(LOG_INFO,"Serveur en attente de connexion sur le port ...");
    // Accepter une seule connexion
    int *sockclient;    
    while (1)
    {
        sockclient=(int*)malloc(sizeof(int));
        accept_client(sock,sockclient);
        debug_print("Connexion établie avec le client");
        pthread_t thread_client;
        pthread_create(&thread_client,NULL,(void*)integre_player,sockclient);
    }

}
