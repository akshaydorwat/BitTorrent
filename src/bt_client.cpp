#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h> //ip hdeader library (must come before ip_icmp.h)
#include <netinet/ip_icmp.h> //icmp header
#include <arpa/inet.h> //internet address library
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#include "bt_lib.h"
#include "bt_setup.h"

// CPP libraries
#include "Reactor.hpp"
#include "Logger.hpp"

using namespace std;

int main (int argc, char * argv[]){

  bt_args_t bt_args;
  int i;
  ofstream log_file;

  parse_args(&bt_args, argc, argv);

  if(bt_args.verbose){
    printf("Args:\n");
    printf("verbose: %d\n",bt_args.verbose);
    printf("save_file: %s\n",bt_args.save_file);
    printf("log_file: %s\n",bt_args.log_file);
    printf("torrent_file: %s\n", bt_args.torrent_file);

    for(i=0;i<MAX_CONNECTIONS;i++){
      if(bt_args.peers[i] != NULL)
        print_peer(bt_args.peers[i]);
    }
  }

  // Register logging file wih Logger 
  Logger* l = Logger::getInstance();
  l->addOutputStream((ostream)ofstream(bt_args.save_file,std::ofstream::out | std::ofstream::app), INFO, string("%F %T"));  

  Reactor* r = Reactor::getInstance();
  
  
  

  l->LOG(INFO, "Log is working");

  //  while(1){

    //try to accept incoming connection from new peer
       
    
    //poll current peers for incoming traffic
    //   write pieces to files
    //   udpdate peers choke or unchoke status
    //   responses to have/havenots/interested etc.
    
    //for peers that are not choked
    //   request pieaces from outcoming traffic

    //check livelenss of peers and replace dead (or useless) peers
    //with new potentially useful peers
    
    //update peers, 

  //}

  return 0;
}
