#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../lib/microtcp.h"
#include <time.h>
#include <unistd.h>

int main(int argc, char **argv){
microtcp_sock_t sock;
int len;
microtcp_header_t recv_Packet_Client,packet1,packet2;
    microtcp_header_t FIN_Packet_Client,ACK_Packet_Client;
    int X,Y;
    struct sockaddr_in sin;
    socklen_t sin_len;
    char*buffer;
    /*DHMIOURGIA TOU SOCKET*/
    sock=microtcp_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    /*DHMIOURGIA THS DIEUTHUNSHS*/
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(54321);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin_len = sizeof(struct sockaddr_in);
    sock.addr=sin;
    sock.addr_len=sin_len;
    /*CONNECT TO THE SERVER*/
    if (microtcp_connect(&sock, (struct sockaddr *)&sin, sin_len) == -1) {
        perror("CONNECT FAILED");
        exit(EXIT_FAILURE);
    }   
    buffer="anna";
    len=strlen(buffer);
    if(microtcp_send(&sock,buffer,len,0)==-1){
        perror("microtcp_send failed.\n");
    }
    buffer="makridou";
    len=strlen(buffer);
    if(microtcp_send(&sock,buffer,len,0)==-1){
        perror("microtcp_send failed.\n");
    }
    buffer="noriana";
    len=strlen(buffer);
    if(microtcp_send(&sock,buffer,len,0)==-1){
        perror("microtcp_send failed.\n");
    }
    buffer="xhaxhaj";
    len=strlen(buffer);
    if(microtcp_send(&sock,buffer,len,0)==-1){
        perror("microtcp_send failed.\n");
    }
    //BEGIN THE SHUTDOWN
    microtcp_shutdown(&sock,0);
     return 0;
}
