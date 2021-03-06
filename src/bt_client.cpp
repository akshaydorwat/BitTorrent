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
#include "TorrentCtx.hpp"
#include "Logger.hpp"

using namespace std;

int main (int argc, char * argv[]){

  bt_args_t bt_args;
  ofstream log_file;
  char inp;

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

  // Initialise reactor reactor
  Reactor* reactor = Reactor::getInstance();
  reactor->setPortRange(INIT_PORT, MAX_PORT);
  reactor->setMaxConnections(MAX_CONNECTIONS);
  reactor->setPollTimeOut(POLL_TIMEOUT);
  reactor->setScocketAddr(bt_args.sockaddr);
  reactor->initReactor();

  // Load Torrent Context
  TorrentCtx t; 

  // Initialise the context
  t.init(&bt_args);
  reactor->setTorrentCtx(&t);
  reactor->startReactor();
    cout << "------------------ Enter Q or q (followed by ENTER key) to quit --------------------" << endl;
  while( inp != 'Q' && inp != 'q'){
    inp = getchar();
    cout << "------------------ Enter Q or q (followed by ENTER key) to quit --------------------" << endl;
  }
  if (inp == 'Q' || inp == 'q')
  	LOG (INFO, "bt_client : Terminating by user command. Please wait ...");
  else
	LOG (INFO, "bt_client : Terminating by preset condition. Please wait ...");
  
  reactor->closeReactor();
  return 0;
}
