#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include "../lib/microtcp.h"


int main(void) {
    microtcp_sock_t sock;
    struct sockaddr_in sin;
    socklen_t sin_len;
    microtcp_header_t recv_Packet_Server,FIN_Packet_Server,ACK_Packet_Server;
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
    /*BIND SOCKET TO THE ADDRESS*/
    if (microtcp_bind(&sock, (struct sockaddr *)&sin, sin_len) == -1) {
        perror("Bind Failed.\n\n");
        exit(EXIT_FAILURE);
    }
    /*ACCEPT CLIENT'S REQUEST FOR CONNECTION*/
    if (microtcp_accept(&sock, (struct sockaddr *)&sin, sin_len) == -1) {
        perror("Connect Failed.\n\n");
        exit(EXIT_FAILURE);
    }
    /*WAITING FOR CLIENT TO SENT SOMETHING*/
    sock.recvbuf=(uint8_t *)malloc(MICROTCP_RECVBUF_LEN);  /*Allocate xwro sth mnhmh gia ton buffer tou server pou periexei antikeimena tupou microtcp_header_t*/
    if(!sock.recvbuf)return -1;
    microtcp_recv (&sock,sock.recvbuf,MICROTCP_RECVBUF_LEN,0);
    microtcp_recv (&sock,sock.recvbuf,MICROTCP_RECVBUF_LEN,0);
    microtcp_recv (&sock,sock.recvbuf,MICROTCP_RECVBUF_LEN,0);
    microtcp_recv (&sock,sock.recvbuf,MICROTCP_RECVBUF_LEN,0);
    microtcp_recv (&sock,sock.recvbuf,MICROTCP_RECVBUF_LEN,0);
    if(sock.state==CLOSING_BY_PEER)microtcp_shutdown(&sock,0);
    return 0;
}
