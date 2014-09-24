/*==============================
   Author : Akshay Dorwat
   Email ID: adorwat@indiana.edu
   created on: 09/22/2014
=================================*/

#ifndef REACTOR_H
#define REACTOR_H


#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#include <iostream>
#include <map>
#include <vector>
#include <mutex>
#include "Peer.hpp"
#include "bt_lib.h"

using namespace std;

class Reactor{

public:

  // Init reactor
  void initReactor();
  
  // Register Events with Reactor
  void registerEvent(int fd, Peer* peer);

  // Remove event from register
  void unRegisterEvent(int fd);

  // Reactor loop
  void loopForever();

  // scan the regiter
  Peer* scanEventRegister(int key);

  // set port range to try while binding
  void setPortRange(unsigned short min_p, unsigned short max_p){
    min_port = min_p;
    max_port = max_p;
  }

  // set socketaddr for Server
  void setScocketAddr(struct sockaddr_in add){
    addr = add;
  }

  // set poll timeout
  void setPollTimeOut(int timeout){
    poll_timeout = timeout;
  }

  // set connection timeout
  void setConnectionTimeout(int timeout){
    connection_timeout = timeout;
  }

  // set max connections on server
  void setMaxConnections(int num_connection){
    max_connections = num_connection;
  }

  // return reactor status
  bool isReactorStarted(){
    return is_started;
  }

  //start reactor on thread
  bool startReactor();

  // wait till reactor exit
  void wait();

  // Singleton method to getinstance of reactor
  static Reactor* getInstance();

private:

  //  Handle events on socket connections
  void handleEvent();
  
  // shut down reactor
  int closeReactor();

  // fill the pollfd vector with socket descriptor and events
  void fillPollFd();
  
  // thread helper
  static void* threadHelper(void *obj_ptr);

  static Reactor* reactor;
  pthread_t thread;                      // Thread to run reactor
  mutex m_lock;                          // Serialize access to eventRegister
  bool is_started;                       // Server on/off
  unsigned short min_port;               // Port range to try  connect on server
  unsigned short max_port;              
  struct sockaddr_in addr;               // sockaddr for server
  int server_sfd;                        // Server socket descriptor
  int max_connections;                    // maximum number of connection supported on the server
  int poll_timeout;                      // Polling timeout in milisec
  int connection_timeout;                // Connection timeout for each socket
  vector<pollfd> poll_fd;                // Polling fds
  map<int,Peer*> eventRegister;
};

#endif



