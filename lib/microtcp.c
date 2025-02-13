/*
 * microtcp, a lightweight implementation of TCP for teaching,
 * and academic purposes.
 *
 * Copyright (C) 2015-2017  Manolis Surligas <surligas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "microtcp.h"
#include "../utils/crc32.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include<string.h>
#include <unistd.h>
#include <ctype.h>

#define ACK (0b1<<12)
#define SYN (0b1<<14)
#define FIN (0b1<<15)

/*-------PHASE B-------*/
struct SentPackets* headC=NULL;

void FreeClientList() {
    struct SentPackets* current = headC;
    struct SentPackets* next;
    while (current != NULL) {     /*Apeleutherwsh listas*/
        next = current->next;
        free(current);
        current = next;
    }
    headC = NULL;  
}

/*Sunarthsh pou eisagei paketa pou stelnontai apo ton client sth lista*/
int InsertClient(struct SentPackets* current,int x,int y,int z) {  
    struct SentPackets* newnode;
    struct SentPackets* last;
    newnode=(struct SentPackets*)malloc(sizeof(struct SentPackets));
    if (!newnode) {
      perror("Memory allocation failed.\n");    
      return 0;
    }
    last=headC;
    newnode->seq=x;
    newnode->size=y;
    newnode->num=z;
    newnode->next=NULL;                                        
    if(current==NULL) {    /*an h kefalh einai NULL tote to newnode einai to head ths listas*/
        newnode->next=headC;                          
        headC=newnode;
    }
    else{ 
        while(last->next!=NULL) {   /*alliws vazw to stoixeio sto telos ths listas. vriskw to telos ths listas me to last pointer*/
            last=last->next;
        }       
        last->next=newnode;   /*teleutaio stoixeio to newnode*/
    }
    return 0;
}

/*Tupwnei thn teleutaia eisagwgh pou egine sth lista*/
void PrintClient(const void* msg) {
    struct SentPackets* current;
    char* ptr = (char*) msg;
    current=headC;
    while (current->next!= NULL) {        /*diasxizw olo th lista kai tupwnw to teleutaio node*/
        current=current->next;
    }
    printf("Sent Data: %s with SEQ: %d and Size: %d.\nExpected ACK: %d \n\n",ptr,current->seq,current->size,current->seq+current->size);
}


/*-------PHASE A-------*/
microtcp_sock_t microtcp_socket (int domain, int type, int protocol){
  microtcp_sock_t sock;
  /*Dhmiourgia tou socket*/
  if((sock.sd = socket(domain, type, protocol)) == -1) {
    perror("Socket Failed.\n\n");
    exit(EXIT_FAILURE); 
  }
  /*Arxikopoihsh twn pediwn tou socket structure*/
  sock.state=CLOSED;  /*Arxikh katastash enos socket: CLOSED*/
  sock.init_win_size=0; 
  sock.curr_win_size=0; 
  sock.recvbuf=NULL;
  sock.buf_fill_level=0;
  sock.cwnd=MICROTCP_INIT_CWND; /*B FASH: Arxikopoihsh cwnd, ssthresh*/ 
  sock.ssthresh=MICROTCP_INIT_SSTHRESH; 
  sock.seq_number=0;  
  sock.ack_number=0;  
  sock.packets_send=0;
  sock.packets_received=0;
  sock.packets_lost=0;
  sock.bytes_send=0;
  sock.bytes_received=0;
  sock.bytes_lost=0;
  return sock;
}

int microtcp_bind(microtcp_sock_t *socket, const struct sockaddr *address, socklen_t address_len) {
    if(bind(socket->sd, address, address_len)==-1){ /*Binding the socket*/
        perror("Bind Failed.\n\n"); 
        socket->state=INVALID;  /*An apetuxe h bind tupwnoume error*/
        return -1;
    }
    socket->state=LISTEN; /*Meta to bind to socket mas einai etoimo na lavei connections, giauto thetoume to state se LISTEN*/
    return 0;
}

int microtcp_connect (microtcp_sock_t *socket, const struct sockaddr *address,socklen_t address_len){
    microtcp_header_t recv_Packet;
    microtcp_header_t SYN_Packet,ACK_Packet;
    uint32_t received_checksum, calculated_checksum;
    int N;
    /*VHMA 1 SEND SYN, SEQ=N, N random*/
    printf("\nSTARTING 3 WAY HANDSHAKE..\n");
    srand(time(NULL));
    N=rand()%150+1; /*N from 0 to 150 gia na apofugoume megala noumera*/
    SYN_Packet.control=SYN; /*Thetoume to SYN bit=1*/
    SYN_Packet.seq_number=N; /*Vazoume sto seq_number ton random N*/
    socket->init_win_size=MICROTCP_WIN_SIZE; /*B FASH: arxikopoihsh tou window size apth meria tou client*/
    SYN_Packet.window=socket->init_win_size; /*antallagh tou window size me ton client*/
    socket->curr_win_size=MICROTCP_WIN_SIZE;
	  if(sendto(socket->sd,&SYN_Packet,sizeof(SYN_Packet),0,(struct sockaddr*)address, address_len)!=-1){      
    printf("\nClient sent SYN with SEQ N:<%d> and the window size is: %d\n\n",N,SYN_Packet.window);  /*Stelnoume ston server to SYN paketo pou periexei to sequence number*/
	  socket->state=SYN_SENT;
    }
    else {
      perror("sendto Failed.\n\n");
    }
    /*VHMA 2 RECEIVE SYNACK, SEQ=M, ACK=N+1*/
    socket->recvbuf=(uint8_t *)malloc(MICROTCP_RECVBUF_LEN);  /*Allocate xstruct SentPackets* o sth mnhmh gia ton buffer tou client pou periexei antikeimena tupou microtcp_header_t*/
    if(!socket->recvbuf)return -1;
	  if(recvfrom(socket->sd,socket->recvbuf,1024,0,(struct sockaddr*)address, &address_len)!=-1){      
      memcpy(&recv_Packet, socket->recvbuf, sizeof(microtcp_header_t)); /*Afou lavoume to paketo to apothikeuoume se mia metavlhth tupou microtcp_header_t*/  
      if(recv_Packet.control==(SYN|ACK) && recv_Packet.ack_number==SYN_Packet.seq_number+1){ /*Elegxoume an elave to SYNACK o client me to swsto ACK*/
        printf("Client received SYNACK with SEQ=M:<%d> and ACK=N+1:<%d> and Server's window size: %d\n\n",recv_Packet.seq_number,recv_Packet.ack_number, recv_Packet.window); /*Lamvanoume apo ton server to SYNACK paketo*/
        if(socket->cwnd<=socket->ssthresh){
          socket->cwnd=socket->cwnd+MICROTCP_MSS;   /*swsto ACK kai slow start, cwnd=cwnd+MSS*/
        }
      }
      else{
        socket->state=INVALID;  /*An apetuxe tupwnoume error*/
        perror("Handshake Failed.\n\n");
        
      }
    }
    else{
      socket->state=INVALID;  /*An apetuxe h microtcp_recv tupwnoume error*/
      perror("recvfrom Failed.\n\n");
    }
    sleep(2);
    /*VHMA 3 SEND ACK, SEQ=N+1, ACK=M+1*/
    ACK_Packet.control=ACK; /*Gia na steilei o client to ACK ston server theloume to ACK bit=1*/
    ACK_Packet.ack_number=recv_Packet.seq_number+1; /*To ACK tou client isoutai me to sequence number tou server+1. Ara prosthetoume 1 sto seq pou lavame prin*/
    ACK_Packet.seq_number=SYN_Packet.seq_number+1; /*To seq tou client einai to sequence number tou SYN paketou pou esteile o client sto vhma 1 +1.*/
    ACK_Packet.checksum = crc32((uint8_t*)&ACK_Packet, sizeof(microtcp_header_t));
    if(sendto(socket->sd,&ACK_Packet,sizeof(ACK_Packet),0,(struct sockaddr*)address, address_len)!=-1){
      printf("Client sent ACK with SEQ N+1:<%d> and ACK M+1:<%d>\n\n",ACK_Packet.seq_number,ACK_Packet.ack_number);  /*Stelnoume to ACK ston server*/
    }
    else{
      perror("microtcp_send Failed.\n\n");
    }
    free(socket->recvbuf);  /*Apeleutherwsh desmeumenhs mnhmhs*/
    printf("FINISHING 3 WAY HANDSHAKE..\n\n");
  return 0;
}

int microtcp_accept (microtcp_sock_t *socket, struct sockaddr *address,socklen_t address_len){
    microtcp_header_t recv_Packet,error_Packet;
    size_t seq; 
    uint32_t received_checksum, calculated_checksum;
    microtcp_header_t SYN_Packet,ACK_Packet;
    microtcp_header_t SYNACK_Packet;
    int M;
    /*VHMA 1 RECEIVE*/
    socket->recvbuf=(uint8_t *)malloc(MICROTCP_RECVBUF_LEN);  /*Allocate xstruct SentPackets* o sth mnhmh gia ton buffer tou server pou periexei antikeimena tupou microtcp_header_t*/
    if(!socket->recvbuf)return -1;
    if(recvfrom(socket->sd, socket->recvbuf, 1024, 0, (struct sockaddr*)address, &address_len)!=-1){
      memcpy(&recv_Packet, socket->recvbuf, sizeof(microtcp_header_t)); /*Apothikeuoume se mia metavlhth tupou microtcp_header_t to SYN paketo*/
      if(recv_Packet.control==SYN) {
        printf("\nServer received SYN with SEQ N:<%d> and Client's window size: %d\n\n",recv_Packet.seq_number,recv_Packet.window); /*O server lamvanei to SYNACK paketo*/
        socket->state=SYN_RECEIVED;
        seq=recv_Packet.seq_number;
        if(socket->cwnd<=socket->ssthresh){
          socket->cwnd=socket->cwnd+MICROTCP_MSS;   /*swsto ACK kai slow start, cwnd=cwnd+MSS*/
        }
      }
      else{
        socket->state=INVALID;  /*An apetuxe tupwnoume error*/
        perror("Handshake Failed.\n\n");
      }
    }
    else{
      socket->state=INVALID;  /*An apetuxe h microtcp_recv tupwnoume error*/
      perror("recvfrom Failed.\n\n");
    }
    sleep(2);
    /*VHMA 2 SEND*/
    srand(time(NULL));
    M=rand()%100+1; /*M from 0 to 100 gia na apofugoume megala noumera*/
    SYNACK_Packet.control=SYN|ACK;  /*Thetoume to SYN,ACK bit=1*/
    SYNACK_Packet.seq_number=M; /*To seq number einai enas random arithmos*/
    SYNACK_Packet.ack_number=recv_Packet.seq_number+1; /*To ACK number einai to seq number pou esteile o client(to apothikeusame prin sto recv_Packet)+1*/
    socket->init_win_size=MICROTCP_WIN_SIZE; /*B FASH: arxikopoihsh tou window size apth meria tou server*/
    socket->curr_win_size=MICROTCP_WIN_SIZE;
    SYNACK_Packet.window=socket->init_win_size;  /*antallagh tou window size me ton server*/
    SYNACK_Packet.checksum = crc32((uint8_t*)&SYNACK_Packet, sizeof(microtcp_header_t));
    if(sendto(socket->sd,&SYNACK_Packet,sizeof(SYNACK_Packet),0,(struct sockaddr*)address, address_len)!=-1) {
      printf("Server sent SYNACK with SEQ=M:<%d> and ACK=N+1:<%d> and the window size: %d\n\n",SYNACK_Packet.seq_number,SYNACK_Packet.ack_number, SYNACK_Packet.window); /*Stelnei to SYNACK paketo ston client*/
    }
    else {
      perror("microtcp_send Failed.\n\n");
    }

    /*VHMA 3 RECEIVE*/
    if(recvfrom(socket->sd, socket->recvbuf, 1024, 0, (struct sockaddr*)address, &address_len)!=-1){
      memcpy(&recv_Packet, socket->recvbuf, sizeof(microtcp_header_t)); /*O server lamvanei to ACK tou client*/
      if(recv_Packet.control==ACK && recv_Packet.seq_number==seq+1 && recv_Packet.ack_number==SYNACK_Packet.seq_number+1) {  /*elegxw an einai swsto to seq*/
        socket->state=ESTABLISHED;
        printf("Server received ACK with SEQ N+1:<%d> and ACK M+1:<%d>\n\n",recv_Packet.seq_number,recv_Packet.ack_number);
        /*Me auto oloklhrwnetai h xeirapsia. H sundesh einai ESTABLISHED*/
        if(socket->cwnd<=socket->ssthresh){
          socket->cwnd=socket->cwnd+MICROTCP_MSS;   /*swsto ACK, cwnd=cwnd+MSS*/
        }
      }
      else{
        socket->state=INVALID;  /*An apetuxe tupwnoume error*/
        perror("Handshake Failed.\n\n");
      }
    }
    else{
        socket->state=INVALID;  /*An apetuxe tupwnoume error*/
        perror("microtcp_recv Failed.\n\n");
      }
    free(socket->recvbuf); /*Apeleutherwsh desmeumenhs mnhmhs*/
    printf("CONNECTION ESTABLISHED.\n\n");
    /*B FASH: Desmeuw xstruct SentPackets* o afou teleiwse h egkathidrush gia thn lhpsh twn paketwn me vash to window pou antallaxthke*/
    socket->recvbuf=(uint8_t *)malloc(MICROTCP_RECVBUF_LEN);
    if(!socket->recvbuf)return -1;
    if(MICROTCP_RECVBUF_LEN<MICROTCP_WIN_SIZE) perror("Buffer size should be greater than window size or equal");
    return 0;
}


int microtcp_shutdown (microtcp_sock_t *socket, int how){
  microtcp_header_t recv_Packet_Client;
  microtcp_header_t FIN_Packet_Client,ACK_Packet_Client;
  microtcp_header_t recv_Packet_Server;
  microtcp_header_t FIN_Packet_Server,ACK_Packet_Server;
  uint32_t received_checksum, calculated_checksum;
  int X,Y;
  srand(time(NULL));
  /*MERIA TOU SERVER*/
  if(socket->state==CLOSING_BY_PEER){
    printf("ACCEPTING SHUTDOWN..\n\n");
      /*VHMA 3: SERVER SENDS FINACK TO CLIENT, SEQ=Y*/
      Y=rand()%100+1; /*Y from 1 to 100 gia na apofugoume megala noumera*/
      FIN_Packet_Server.control=FIN|ACK; /*Thetoume to FIN, ACK bit=1*/
      FIN_Packet_Server.seq_number=Y; /*Vazoume sto seq_number ton random Y*/
      FIN_Packet_Server.checksum = crc32((uint8_t*)&FIN_Packet_Server, sizeof(microtcp_header_t));
      if(sendto(socket->sd,&FIN_Packet_Server,sizeof(FIN_Packet_Server),0,(struct sockaddr*)(&socket->addr), socket->addr_len)!=-1) {
        printf("Server sent FIN with SEQ Y:<%d>.\n\n",Y); /*Stelnei to FIN paketo ston client*/
      }
      else {
        perror("sendto Failed.\n\n");
      }
      /*VHMA 3 RECEIVE*/
    if(recvfrom(socket->sd, socket->recvbuf, MICROTCP_RECVBUF_LEN, 0, (struct sockaddr*)(&socket->addr), &socket->addr_len)!=-1) {
        memcpy(&recv_Packet_Server, socket->recvbuf, sizeof(microtcp_header_t)); /*O server lamvanei to ACK tou client*/
        if(recv_Packet_Server.control==ACK && recv_Packet_Server.ack_number==FIN_Packet_Server.seq_number+1) {
          printf("Server received ACK Y+1:<%d> with SEQ X+1:<%d> ",recv_Packet_Server.ack_number,recv_Packet_Server.seq_number);
          if(socket->cwnd<=socket->ssthresh){
            socket->cwnd=socket->cwnd+MICROTCP_MSS;   /*swsto ACK, cwnd=cwnd+MSS*/
          }
        }
        else {
          socket->state=INVALID;    /*An apetuxe h microtcp_send tupwnoume error*/
          perror("Shutdown Failed.\n\n");
        }
      }
      else {
        socket->state=INVALID;    /*An apetuxe h microtcp_send tupwnoume error*/
        perror("recvfrom Failed.\n\n");
      }
      socket->state=CLOSED;
      printf("and updated the state to CLOSED.\n\n");
      sleep(2);
      free(socket->recvbuf); /*Apeleutherwsh desmeumenhs mnhmhs*/
      printf("SERVERS'S SOCKET CLOSED.\n\n");
  }
  /*MERIA TOU CLIENT*/
  else {
    printf("\nSTARTING SHUTDOWN..\n\n");
    /*VHMA 1: CLIENT STARTS SHUTDOWN BY SENDING FIN, SEQ=X*/
      X=rand()%150+1; /*X from 1 to 150 gia na apofugoume megala noumera*/
      FIN_Packet_Client.control=FIN|ACK; /*Thetoume to FIN ACK bit=1*/
      FIN_Packet_Client.seq_number=X; /*Vazoume sto seq_number ton random X*/
      FIN_Packet_Client.checksum = crc32((uint8_t*)&FIN_Packet_Client, sizeof(microtcp_header_t));
      if(sendto(socket->sd,&FIN_Packet_Client,sizeof(FIN_Packet_Client),0,(struct sockaddr*)(&socket->addr), socket->addr_len)!=-1){
        printf("Client sent FIN to server with SEQ X:<%d>\n\n",X);  /*Stelnoume ston server to FIN paketo*/
      }
      else {
        perror("microtcp_send Failed.\n\n");
      }
      /*VHMA 2: SERVER SENDS ACK AND CLIENT RECEIVES IT, THEN SETS STATE TO CLOSING_BY_HOST*/
      socket->recvbuf=malloc(MICROTCP_RECVBUF_LEN);  /*Allocate xstruct SentPackets* o sth mnhmh gia ton buffer tou client pou periexei antikeimena tupou microtcp_header_t*/
      if(!socket->recvbuf)return -1;
      if(recvfrom(socket->sd,socket->recvbuf,1024,0,(struct sockaddr*)(&socket->addr), &socket->addr_len)!=-1){
        memcpy(&recv_Packet_Client, socket->recvbuf, sizeof(microtcp_header_t)); /*Afou lavoume to paketo to apothikeuoume se mia metavlhth tupou microtcp_header_t*/
        if(recv_Packet_Client.control==ACK && recv_Packet_Client.ack_number==FIN_Packet_Client.seq_number+1) {
          printf("Client received ACK X+1:<%d> ",recv_Packet_Client.ack_number); /*Lamvanoume apo ton server to SYNACK paketo*/
          socket->state=CLOSING_BY_HOST;
          printf("and updated the state to CLOSING_BY_HOST.\n\n");
          if(socket->cwnd<=socket->ssthresh){
            socket->cwnd=socket->cwnd+MICROTCP_MSS;   /*swsto ACK, cwnd=cwnd+MSS*/
          }
        }
        else{
          socket->state=INVALID;  /*An apetuxe h microtcp_recv tupwnoume error*/
          perror("Shutdown Failed.\n\n");
        }
      }
      else{
        socket->state=INVALID;  /*An apetuxe h microtcp_recv tupwnoume error*/
        perror("microtcp_recv Failed.\n\n");
      }
      sleep(5);
      /*VHMA 3: SERVER SENDS FIN AND CLIENT RECEIVES IT, THEN SETS STATE TO CLOSED*/
      if(recvfrom(socket->sd,socket->recvbuf,1024,0,(struct sockaddr*)(&socket->addr), &socket->addr_len)!=-1){
        memcpy(&recv_Packet_Client, socket->recvbuf, sizeof(microtcp_header_t)); 
        if(recv_Packet_Client.control==(FIN|ACK)){
          printf("Client received FIN with SEQ Y:<%d>\n\n",recv_Packet_Client.seq_number); 
          if(socket->cwnd<=socket->ssthresh){
            socket->cwnd=socket->cwnd+MICROTCP_MSS;   /*swsto ACK, cwnd=cwnd+MSS*/
          }
        }
        else{
          socket->state=INVALID;  
          perror("Shutdown Failed.\n\n");
        }
      }
      else{
        socket->state=INVALID;  
        perror("microtcp_recv Failed.\n\n");
      }
      socket->state=CLOSED;
      sleep(2);
      /*VHMA 4: CLIENT SENDS ACK PACKET*/
      ACK_Packet_Client.control=ACK; /*Gia na steilei o client to ACK ston server theloume to ACK bit=1*/
      ACK_Packet_Client.ack_number=recv_Packet_Client.seq_number+1; /*To ACK tou client isoutai me to sequence number tou server+1. Ara prosthetoume 1 sto seq pou lavame prin*/
      ACK_Packet_Client.seq_number=FIN_Packet_Client.seq_number+1; /*To seq tou client einai to sequence number tou FIN paketou pou esteile o client sto vhma 1 +1.*/
      ACK_Packet_Client.checksum = crc32((uint8_t*)&ACK_Packet_Client, sizeof(microtcp_header_t));
      if(sendto(socket->sd,&ACK_Packet_Client,sizeof(ACK_Packet_Client),0,(struct sockaddr*)(&socket->addr), socket->addr_len)!=-1){
        printf("Client sent ACK Y+1:<%d> to server with SEQ X+1:<%d> ",ACK_Packet_Client.ack_number,ACK_Packet_Client.seq_number);  /*Stelnoume to ACK ston server*/
      }
      else{
        perror("sendto Failed.\n\n");
      }
      printf("and updated the state to closed.\n\n");
      free(socket->recvbuf);  /*Apeleutherwsh desmeumenhs mnhmhs*/
      FreeClientList();   /*Apeleutherwsh ths listas tou client*/
      printf("CLIENT'S SOCKET CLOSED.\n\n");
  }
return 0;
}

/*-------PHASE B-------*/
ssize_t microtcp_send(microtcp_sock_t *socket, const void *buffer, size_t length, int flags) {
  microtcp_header_t packet, ACK_from_Server;
  uint32_t expectedACK;
  struct SentPackets *last;
  packet.data_len=length;
  int num;
  if(length<=socket->curr_win_size){ /*flow control step 3 - o client mporei na steilei to poly osa bytes epitrepei to window*/
    if (headC ==NULL) {   /*Adeia lista, 0 paketa exoun stalei*/
      num=1;    /*Stelnetai to prwto paketo me random sequence number*/
      packet.seq_number = rand() % 100 + 1;   /*Seq number tuxaios arithmos ews 100*/
      if(length<MICROTCP_MSS) InsertClient(headC, packet.seq_number, packet.data_len, num); /*flow control step 1- eisagwgh 1ou paketou*/
    } 
    else {  /*An h lista den einai adeia, psaxnoume to teleutaio stoixeio ths listas*/
      last=headC;
      if(last!=NULL){
        while (last->next!=NULL) {
          last=last->next;
        } /*To seq number tou paketou pou stelnei o client exei timh ish me to seq tou prohgoumenou+ to size tou*/
        packet.seq_number=last->seq+last->size;
        num=last->num+1;  /*arithmos tou paketou pou esteila*/
        InsertClient(headC, packet.seq_number, packet.data_len, num); 
      }
    }
    /*To ACK pou anamenoume apo ton server einai iso me to sequence number pou tou steilame+ to size tou buffer*/
    expectedACK = packet.seq_number+packet.data_len;
    PrintClient(buffer);

    /*Dhmiourgia paketou pros apostolh*/
    char tmp_buff[sizeof(microtcp_header_t)+length];  /*Ftiaxnoume enan neo buffer*/
    memcpy(tmp_buff, &packet, sizeof(microtcp_header_t));  /*Periexei to header tou paketou*/
    memcpy(tmp_buff + sizeof(microtcp_header_t), buffer, length);  /*Kai ta data tou*/

    /*Apostolh paketou apo ton Client ston Server*/
    socket->bytes_send = sendto(socket->sd, tmp_buff, sizeof(tmp_buff), flags,(struct sockaddr *)(&socket->addr), socket->addr_len);
    if (socket->bytes_send == -1) {
      printf("Client failed to send to Server.\n");
      socket->state = INVALID;
      return -1;
    }
    /*O client prepei prwta na lavei ACK apton server gia na thewrhtei epituxhs h apostolh*/
    if (recvfrom(socket->sd, socket->recvbuf, sizeof(socket->recvbuf), 0,(struct sockaddr *)(&socket->addr), &socket->addr_len) == -1) {
      perror("Client failed to receive ACK.\n");
      return -1;
    } 
    else {
      memcpy(&ACK_from_Server, socket->recvbuf, sizeof(microtcp_header_t));
      if (ACK_from_Server.ack_number != expectedACK) {
        printf("Expecting %d, Wrong ACK received.\n", expectedACK);
      }
      socket->curr_win_size=ACK_from_Server.window; /*flow control step3 - o client mporei na steilei osa bytes tou epitrepei o server*/
    }
  return ACK_from_Server.data_len;    /*Epistrefomenh timh: Epivevaiwmenos arithmos bytes*/
  }
  else {
    perror("Flow control doesnt allow the Client to send this ammount of bytes.\n");
    return -1;
  }
}


ssize_t microtcp_recv(microtcp_sock_t *socket, void *buffer, size_t length, int flags) {
  microtcp_header_t pack, ACK_Packet_Server;
  struct timeval timeout;
  struct SentPackets* ptr;
  uint32_t ack;
  int num;
  timeout.tv_sec = 0;
  timeout.tv_usec = MICROTCP_ACK_TIMEOUT_US;

  /*Lhpsh tou paketou tou Client*/
  socket->bytes_received = recvfrom(socket->sd, buffer, length, flags,(struct sockaddr *)(&socket->addr), &socket->addr_len);
  if (socket->bytes_received == -1) {
    perror("Server failed to receive Client's packet");
    socket->state = INVALID;
    return -1;
  }

  /*Apothikeuoume ton header*/
  memcpy(&pack, buffer, sizeof(microtcp_header_t));

  /*An ta control bits einai FINACK tote kaloume thn microtcp_shutdown*/
  if (pack.control==(FIN|ACK)) {
    printf("\nServer received FIN with SEQ X:<%d>.\n\n", pack.seq_number);
    srand(time(NULL));
    ACK_Packet_Server.control=ACK;
    ACK_Packet_Server.ack_number = pack.seq_number+1;
    if (sendto(socket->sd, &ACK_Packet_Server, sizeof(ACK_Packet_Server), flags,(struct sockaddr *)(&socket->addr), socket->addr_len) != -1) {
      printf("Server sent ACK X+1:<%d> ", ACK_Packet_Server.ack_number);
    } 
    else {
      socket->state = INVALID;
      perror("Sendto Failed.\n\n");
    }

    socket->state = CLOSING_BY_PEER;
    printf("and updated the state to CLOSING_BY_PEER.\n\n");
    return -1;
  } 
  else {  /*an den exoume FINACK tote ftiaxnoume enan neo buffer*/
    char recv_buff[pack.data_len+1]; /* +1 gia to '/0' */
    memcpy(recv_buff, buffer + sizeof(microtcp_header_t), pack.data_len); /*apothikeuoume ta dedomena pou lavame*/
    recv_buff[pack.data_len]='\0';  /*null terminated string*/

    pack.ack_number=pack.seq_number+pack.data_len;  /*to ack einai to seq pou lavame+ to length*/

    printf("Received Data: %s with SEQ: %d.\n", recv_buff,pack.seq_number); /*tupwnoume ta data pou elave o server*/
    
    /*Apostolh ACK ston Client ws epivevaiwsh oti lhfthike to paketo*/
    ACK_Packet_Server.control=ACK;
    ACK_Packet_Server.ack_number=pack.ack_number;
    ACK_Packet_Server.data_len=pack.data_len;

    ACK_Packet_Server.window=ACK_Packet_Server.window-pack.data_len; /*flow control step 2 - stelnei to ack me th nea window timh*/
    
    if (sendto(socket->sd, &ACK_Packet_Server, sizeof(ACK_Packet_Server), flags,(struct sockaddr *)(&socket->addr), socket->addr_len) != -1) {
      printf("Sending ACK: %d.\n\n", ACK_Packet_Server.ack_number);
    } 
    else {
      socket->state = INVALID;
      perror("Server failed to send ACK to client.\n\n");
      return -1;
    }
  }
  return socket->bytes_received;
}

