#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h> //internet address library
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <arpa/inet.h>

#include <openssl/sha.h> //hashing pieces

#include "bt_lib.h"
#include "bt_setup.h"


void calc_id_sockaddr(struct sockaddr_in *addr, char *id){
  // If client id is not provided calculate the client ID                                                         
  char *ip;
  unsigned short port;
  

  if(addr != NULL){
    ip = inet_ntoa(addr->sin_addr);
    port = addr->sin_port ;
    printf("IP: %s Port: %u", ip, port);
    calc_id(ip, port, id);
  }else{
    fprintf(stderr, "[Error] Failed to calculate client Id\n");
    exit(1);
  }
}

void calc_id(char * ip, unsigned short port, char *id){
  char data[256];
  int len;
  
  //format print
  len = snprintf(data,256,"%s%u",ip,port);
  //id is just the SHA1 of the ip and port string
  SHA1((unsigned char *) data, len, (unsigned char *) id); 

  return;
  }


/**
 * init_peer(peer_t * peer, int id, char * ip, unsigned short port) -> int
 *
 *
 * initialize the peer_t structure peer with an id, ip address, and a
 * port. Further, it will set up the sockaddr such that a socket
 * connection can be more easily established.
 *
 * Return: 0 on success, negative values on failure. Will exit on bad
 * ip address.
 *   
 **/
int init_peer(peer_t *peer, char * id, char * ip, unsigned short port){
    
  struct hostent * hostinfo;
  //set the host id and port for referece
  memcpy(peer->id, id, ID_SIZE);
  peer->port = port;
    
  //get the host by name
  if((hostinfo = gethostbyname(ip)) ==  NULL){
    perror("gethostbyname failure, no such host?");
    herror("gethostbyname");
    exit(1);
  }
  
  //zero out the sock address
  bzero(&(peer->sockaddr), sizeof(peer->sockaddr));
      
  //set the family to AF_INET, i.e., Iternet Addressing
  peer->sockaddr.sin_family = AF_INET;
    
  //copy the address to the right place
  bcopy((char *) (hostinfo->h_addr), 
        (char *) &(peer->sockaddr.sin_addr.s_addr),
        hostinfo->h_length);
    
  //encode the port
  peer->sockaddr.sin_port = htons(port);

  //initialize the chocked and intrested
  peer->choked = 1;
  peer->interested = 0;

  return 0;

}

/**
 * print_peer(peer_t *peer) -> void
 *
 * print out debug info of a peer
 *
 **/
void print_peer(peer_t *peer){
  int i;

  if(peer){
    printf("peer: %s:%u ",
           inet_ntoa(peer->sockaddr.sin_addr),
           peer->port);
    printf("id: ");
    for(i=0;i<ID_SIZE;i++){
      printf("%02x",peer->id[i]);
    }
    printf("\n");
  }
}

void print_peer_id(unsigned char *peerId ){
  int i;
  for(i=0;i<ID_SIZE;i++){
    printf("%02x",peerId[i]);
  }
  printf("\n");
}

void getPeerId(unsigned char *peerId, char *peerIdOut)
{
	int i;
  	for(i=0;i<ID_SIZE;i++)
    		snprintf(&peerIdOut[i*2], 3, "%02x",peerId[i]);
}

void getPeerIpPortId(peer_t *peer, char *peerIp, char *peerPort, char *peerId)
{
	int i;
	if (peer)
	{
		snprintf(peerIp, 16, "%s", inet_ntoa(peer->sockaddr.sin_addr));
		snprintf(peerPort, 6, "%u", peer->port);
		
		for(i=0;i<ID_SIZE;i++)
    			snprintf(&peerId[i*2], 3, "%02x", peer->id[i]);
	}
}
