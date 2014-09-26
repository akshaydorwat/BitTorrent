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
#include "Torrent_t.h"

using namespace std;

int main (int argc, char * argv[]){

  bt_args_t bt_args;
  int i;
  ofstream log_file;

  parse_args(&bt_args, argc, argv);
  
  // Register logging file wih Logger 
  Logger* l = Logger::getInstance();
  log_file.open(bt_args.log_file, ios::out | ios::app);
  log_file << "test";
  l->addOutputStream(&log_file, INFO, string("%F %T"));  

  if(bt_args.verbose){
    l->addOutputStream(&cout, INFO, string("%F %T"));
  }

  LOG(INFO, "verbose:" + to_string(bt_args.verbose));
  LOG(INFO, "save_file:" + string(bt_args.save_file));
  LOG(INFO, "log_file: " + string(bt_args.log_file));
  LOG(INFO, "torrent_file" + string(bt_args.torrent_file));

  // parse torrent file and create context.
  Torrent_t::decode(string(bt_args.torrent_file));

  // start reactor
  Reactor* reactor = Reactor::getInstance();
  reactor->setPortRange(INIT_PORT, MAX_PORT);
  reactor->setMaxConnections(MAX_CONNECTIONS);
  reactor->setPollTimeOut(POLL_TIMEOUT);
  reactor->setScocketAddr(bt_args.sockaddr);
  reactor->initReactor();
  reactor->startReactor();
  reactor->wait();

  //TODO: Put loopforever on thread 

  //start Peers

  //
  // create seession for torrent file we parsed
  // register events with session
  // create processing loop
  // File handlers
  // Peers
  

  /*  for(i=0;i<MAX_CONNECTIONS;i++){
      if(bt_args.peers[i] != NULL)
        print_peer(bt_args.peers[i]);
    }
    }*/


  //Reactor* r = Reactor::getInstance();



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
