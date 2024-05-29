#include "utils.h"

void debug_print(char * mes){
    if(DEBUG)
        printf("%s\n",mes);
}

Message_header create_header(uint16_t codereq, uint16_t id, uint16_t eq) {
    // Assurez-vous que les valeurs sont dans la plage correcte
    codereq = codereq & 0x1FFF; // CODEREQ doit être sur 13 bits
    id = (id & 0x03) << 13;     // ID est sur 2 bits et décalé de 13 bits
    eq = (eq & 0x01) << 15;     // EQ est sur 1 bit et décalé de 15 bits

    // Combine les parties en utilisant l'opérateur OR
    uint16_t temp =codereq | id | eq;

    return htons(temp);
}

Message_action create_action(uint16_t num_action,uint16_t action){

    num_action = num_action & 0x1FFF; // num_action doit être sur 13 bits
    action = (action & 0x07) << 13;   // action est sur 3 bits et décalé de 13 bits

    uint16_t temp =num_action | action;

    return htons(temp);

}

/**
*   decode the Message_action struct and fill the num and action param
*   @param action : the uint16_t to decode
*   @param num : pointer to uint16_t, will be filled with the packet number
*   @param act : pointer to uint16_t, will be filled with the action number
*   @author Leo
*/
int decode_action(Message_action action,uint16_t* num, uint16_t* act ){
    action = ntohs(action);
    *num = action & 0x1FFF;
    *act = (action >> 13) & 0x07;
    return 0;

}

//decode the header of the message from uint16_t to the 3 param
int decode_message(uint16_t message, uint16_t *codereq, uint16_t *id, uint16_t *eq) {
    message =ntohs(message);
    *codereq = message & 0x1FFF;        // Obtenir les 13 premiers bits pour CODEREQ
    *id = (message >> 13) & 0x03;       // Décaler de 13 bits vers la droite et obtenir 2 bits pour ID
    *eq = (message >> 15) & 0x01;       // Décaler de 15 bits vers la droite et obtenir 1 bit pour EQ
    return 0;
}

//create a message_tcp struct from the argument passed
Message_TCP create_tcp_message(Message_header header, char * mes){
    
    Message_TCP message;
    message.header = header;
    message.len=strlen(mes);

    strcpy(message.data,mes);
    return message;
}
Message_UDP_action create_udp_message(Message_header header, Message_action action){

    Message_UDP_action message;
    message.header=header;
    message.action=action;
    return message;
}


//teacher's strtol implementation
uint8_t tstrtol_8(char *snb){
	uint8_t hex;
	for(int i=0; i<2; i++){
		uint8_t c = *(snb+i);
		if ('0' <= c && c <= '9')
			c -= '0';
		else if (c != ':' && 'A' < c && c < 'Z')
			c -= 'A' - 10;
		else if (c != ':' && 'a' < c && c < 'z')
			c -= 'a' - 10;
		if(i == 0)
			hex = c << 4;
		else
			hex += c;
	}
	
	return hex;
}

//convert an ipv6 string into a unit8_t array, needs to be freed after use
int chtoint8(char * ipv6,uint8_t* ip_mdiff){
    if(strlen(ipv6)!=39)
        return 1; //error string too long
    
    char * pt = ipv6;
    //at each iteration takes 2 char and convert them int uint8_t, skip the ':'
    for(int i=0;i<16;i++){
        char temp[2];
        /*
        if(DEBUG)
            printf("start: %c, remaining: %s\n",*pt,pt);
        */

        memmove(temp,pt,2);
        ip_mdiff[i]=tstrtol_8(temp);
        pt+=2;
        if(*pt==':')
            pt++;
    }
    return 0;
}

//return a string based on an uint8_t array for ip conversion 
char * int8toch(uint8_t * ipv6){
    char*res = malloc(sizeof(char)*40);
    char *temp = res;
    memset(res,0,40);
    
    for(int i=0;i<16;i++){
       
        sprintf(res,"%02x",ipv6[i]);
        res+=2;
        if(i%2!=0&&i!=15){
            sprintf(res,":");
            res++;
        }
        
        /*if(DEBUG)
            printf("%s\n",temp);*/
        
    }
    return temp;   
}

//create a Integer_parite from the arguments passed
Integr_partie join_new_game(Message_header header,uint16_t port_udp,uint16_t port_mdiff,char * ipv6){
    Integr_partie partie;
    partie.header = header;
    partie.port_mdiff=htons(port_mdiff);
    partie.port_udp=htons(port_udp);

    if(chtoint8(ipv6,partie.ip_mdiff)){
        fprintf(stderr,"error with ipv6 string format\n");
        exit(1);
    }
    return partie;

}

char * new_mdiff_adress() {

    char* existing=strdup("ff12:f140:1f10:0000:0000:0000:0000:0001");
    if (existing==NULL)
    {
        perror("Memory allocation failled");
        pthread_exit((void*)EXIT_FAILURE);
    }
    
    // Generate random hexadecimal characters for the existing address
    for (int i = 15; i < 39; i += 4) {
        for (int j = 0; j < 4; j++) {
            int r = rand() % 10; // Generate random number between 0 and 15 (little bug, changed to 0-9 for the moment)
            char hex_digit = (r < 10) ? (char)(r + '0') : (char)(r - 10 + 'a'); // Convert to hexadecimal digit
            //printf("%c\n", hex_digit);
            existing[i + j] = hex_digit;
        }
        if (i != 35)
            i++;
    }

    return existing;
}

int new_udp_port(int port){
    return port+=2;
}
/**
 * ayoub
*/
void* my_malloc(size_t bytes){       
    void * tmp = malloc(bytes);
    if(tmp == NULL){
        perror("MALLOC FAILED");
        exit(EXIT_FAILURE);
    }
    return tmp; 
} 
/**
 * ayoub
*/
char* serialize_Mdiff(Mdiff_stat stat,size_t* len){
    int size_of_packet = 2 + 2 + 1 + 1 + (stat.largeur * stat.hauteur); //-2
    char* chat_ser = my_malloc(size_of_packet); 
    int offset = 0;
    memcpy(chat_ser,&stat.header,2);
    offset +=2;
    memcpy(chat_ser+offset,&stat.num,2);
    offset +=2;
    memcpy(chat_ser+offset,&stat.hauteur,1);
    offset +=1;
    memcpy(chat_ser+offset,&stat.largeur,1);
    offset +=1;
    memcpy(chat_ser+offset,stat.cases,stat.largeur * stat.hauteur);
    offset += stat.largeur * stat.hauteur;
    *len = offset;
    return chat_ser;
}
/**
 * ayoub

*/
Mdiff_stat deserialize_Mdiff(char* char_msg){
    Mdiff_stat stat;
    memset(&stat,0,sizeof(Mdiff_stat));
    size_t offset = 0;
    memcpy(&stat.header, char_msg + offset, sizeof(stat.header));
    offset += 2;
    memcpy(&stat.num, char_msg + offset, sizeof(stat.num));
    offset += 2;
    memcpy(&stat.hauteur, char_msg + offset, sizeof(stat.hauteur));
    offset += 1;
    memcpy(&stat.largeur, char_msg + offset, sizeof(stat.largeur));
    offset += 1;
    stat.cases=my_malloc(sizeof(uint8_t)*stat.hauteur*stat.largeur+1);
    memcpy(stat.cases, char_msg + offset, (stat.hauteur* stat.largeur)*sizeof(uint8_t));
    return stat;
}
/**
*   same function adapted for partial
*   @author Leo
*/
char* serialize_partial(Partial_grid partial,size_t* len){
    int size_of_packet = 2 + 2 + 1 + (partial.nb*3); 
    char* chat_ser = my_malloc(size_of_packet); 
    int offset = 0;
    memcpy(chat_ser,&partial.header,2);
    offset +=2;
    memcpy(chat_ser+offset,&partial.num,2);
    offset +=2;
    memcpy(chat_ser+offset,&partial.nb,1);
    offset +=1;
    for(int i = 0 ; i < partial.nb ; i++){
        memcpy(chat_ser+offset , &partial.cases[i].ligne , 1 );
        offset+=1;
        memcpy(chat_ser+offset , &partial.cases[i].colonne , 1 );
        offset+=1;
        memcpy(chat_ser+offset , &partial.cases[i].contenu , 1 );
        offset+=1;
    }
    
    *len = offset;
    return chat_ser;
}
/**
*   same function adapted for partial
*   @author Leo
*/
Partial_grid deserialize_partial(char* char_msg){
    Partial_grid partial;
    memset(&partial,0,sizeof(Partial_grid));
    size_t offset = 0;
    memcpy(&partial.header, char_msg + offset, sizeof(partial.header));
    offset += 2;
    memcpy(&partial.num, char_msg + offset, sizeof(partial.num));
    offset += 2;
    memcpy(&partial.nb, char_msg + offset, sizeof(partial.nb));
    offset += 1;
    partial.cases=my_malloc(sizeof(Case)*partial.nb);

    for(int i = 0 ; i < partial.nb ; i++){
        memcpy(&partial.cases[i].ligne,char_msg+offset , 1);
        offset+=1;
        memcpy(&partial.cases[i].colonne,char_msg+offset , 1);
        offset+=1;
        memcpy(&partial.cases[i].contenu,char_msg+offset , 1);
        offset+=1;
    }
    return partial;
}


/**
 * ayoub
*/
void print(LogLevel level, const char *format, ...) {
    if(DEBUG){
        va_list args;
            va_start(args, format);

            switch (level) {
                case LOG_INFO:
                    printf(ANSI_COLOR_BLUE "[INFO]: ");
                    break;
                case LOG_WARNING:
                    printf(ANSI_COLOR_YELLOW "[WARNING]: ");
                    break;
                case LOG_ERROR:
                    printf(ANSI_COLOR_RED "[ERROR]: ");
                    break;
                default:
                    printf("[UNKNOWN]: ");
                    break;
            }

            vprintf(format, args);
            printf(ANSI_COLOR_RESET "\n");

            va_end(args);
    }
    
}

/**
 * void trace_sent(char* packet,size_t packet_size,int thread_id){
    char* filename = NULL;
    char* transfer_mode = NULL;
    switch(get_opcode(packet)){
        case RRQ:{
            filename = get_file_name(packet);
            transfer_mode= get_mode(packet);
            if(thread_id != -1)printf("THREAD # %d ",thread_id);
            printf("sent RRQ <file=%s, mode=%s>\n",filename,transfer_mode);
            free(filename);
            free(transfer_mode);
            break;
        }
        case WRQ:{
            filename = get_file_name(packet);
            transfer_mode = get_mode(packet);
            if(thread_id != -1)printf("THREAD # %d ",thread_id);
            printf("sent WRQ <file=%s, mode=%s>\n",filename,transfer_mode);
            free(filename);
            free(transfer_mode);
            break;
        }
        case ACK:{
            if(thread_id != -1)printf("THREAD # %d ",thread_id);
            printf("sent ACK <block=%d>\n",get_block_number(packet));
            break;
        }
        case DATA:{
            if(thread_id != -1)printf("THREAD # %d ",thread_id);
            printf("sent DATA <block=%d, %ld bytes>\n",get_block_number(packet),packet_size);
            break;
        }
        case ERROR:{
            char* error_msg = get_error_message(packet);
            if(thread_id != -1){
                printf("THREAD # %d ",thread_id);
                }
            printf("sent ERROR <code=%d, msg=%s>\n",get_error_code(packet),error_msg);
            free(error_msg);
            break;
        }
        default:
            break;
    }
}

void trace_received(char* packet,size_t packet_size,int thread_id){
    char* filename = NULL;
    char* transfer_mode = NULL;
    char* error_msg = NULL;
    switch(get_opcode(packet)){
        case RRQ:
            filename = get_file_name(packet);
            transfer_mode= get_mode(packet);
            if(thread_id != -1)printf("THREAD # %d ",thread_id);
            printf("received RRQ <file=%s, mode=%s>\n",filename,transfer_mode);
            free(filename);
            free(transfer_mode);
            break;
        case WRQ:
            filename = get_file_name(packet);
            transfer_mode = get_mode(packet);
            if(thread_id != -1)printf("THREAD # %d ",thread_id);
            printf("received WRQ <file=%s, mode=%s>\n",filename,transfer_mode);
            free(filename);
            free(transfer_mode);
            break;
        case ACK:
            if(thread_id != -1)printf("THREAD # %d ",thread_id);
            printf("received ACK <block=%d>\n",get_block_number(packet));
            break;
        case DATA:
            if(thread_id != -1)printf("THREAD # %d ",thread_id);
            printf("received DATA <block=%d, %ld bytes>\n",get_block_number(packet),packet_size);
            break;
        case ERROR:
            error_msg = get_error_message(packet);
            if(thread_id != -1)printf("THREAD # %d ",thread_id);
            printf("received ERROR <code=%d, msg=%s>\n",get_error_code(packet),get_error_message(packet));
            free(error_msg);
            break;
        default:
            break;
    }
}
*/