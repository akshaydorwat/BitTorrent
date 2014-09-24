/*==============================
   Author : Akshay Dorwat
   Email ID: adorwat@indiana.edu
   created on: 09/22/2014
=================================*/

#include "Reactor.hpp"
#include "Logger.hpp"
#include "Peer.hpp"

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

using namespace std;

// Intialise the global pointer to NULL
Reactor* Reactor::reactor = NULL;

// scan the regiter
Peer* Reactor::scanEventRegister(int key){
  Peer* p;

  map<int,Peer*>::iterator it =  eventRegister.find(key);
  if(it == eventRegister.end()){
    return NULL;
  }else{
    p = it->second;
    return p;
  }
}

/*
 Function Name: getInstance
 paramer : None
 Output  : Reactor global instance
*/
Reactor* Reactor::getInstance(){

  if(reactor == NULL){
    reactor = new Reactor;
  }
  return reactor;
}

void Reactor::initReactor(){
  
  unsigned short i;
  int on = 1 ;
  is_started = false;
  LOG(INFO, "Min Port : " + to_string(min_port));
  LOG(INFO, "Max Port : " + to_string(max_port));
  LOG(INFO, "Max Connections : " + to_string(max_connections));
  LOG(INFO, "Poll Timeout : " + to_string(poll_timeout));
  // Bind to local address and try different ports in port range
  for(i = min_port; i <= max_port; i++){

    // Port to try
    LOG(DEBUG,"trying port : " + to_string(i));
    addr.sin_port = htons(i);
    // Init socket
    if((server_sfd = socket(addr.sin_family, SOCK_STREAM, 0)) == -1){
      LOG(ERROR, "Failed to initialize socket for incoming connections.");
      exit(EXIT_FAILURE);
    }
    // Accept connection from any ip.
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //Set socket to be NON_BLOCKING. 
    //All connection accepted on this socket will be NON-BLOCKING as well
    if(ioctl(server_sfd, FIONBIO, (char *)&on) < 0){
      LOG(ERROR, "Failed to set server socket NON-BLOCKING");
      close(server_sfd);
      exit(EXIT_FAILURE);
    }
    // Bind to local ip address and port
    if(bind(server_sfd, (struct sockaddr *)&(addr), sizeof(sockaddr)) < 0){
      LOG(WARNING, "Failed to bind to  port : " + to_string(i));
      continue;
    }
    // Start listening on the above address
    if (listen(server_sfd, max_connections) < 0){
      LOG(ERROR, "Failed to listen on server address.");
      exit(EXIT_FAILURE);
    }
    break;
  }

  LOG(INFO, "Server listening for client connections on port : " + to_string(i));
  // create poll fd vector 
  fillPollFd();
}

void Reactor::handleEvent(){
  
  int tsfd; //tsfd = transfer-socket fd
  int numBytesRcvd;
  Peer* peer;
  char packet_rcvd[MAX_PACKET_SIZE];
  struct sockaddr_in srcaddr;
  socklen_t srcaddr_len = sizeof(srcaddr);
  const int on = 1;

  for (vector<pollfd>::iterator it = poll_fd.begin() ; it != poll_fd.end(); ++it){

    pollfd p_fd = *it;
    //    LOG(INFO,"checking on socket : "+ to_string(p_fd.fd));
    // Server fd have different handling 
    if(p_fd.fd == server_sfd){
      // Check server for hang up or error
      if(p_fd.events & (POLLHUP|POLLERR)){
	LOG(ERROR,"Server Failed, Tearing Down all connections");
	closeReactor();
      }else if(p_fd.events & (POLLIN)){
	//accept all connections
	do{
	  if ((tsfd = accept(p_fd.fd, (struct sockaddr *)&srcaddr, &srcaddr_len)) < 0){
	    //LOG(WARNING, "Failed to accept client connection request.");
	    break;
	  }
	  LOG(INFO,"Accepted New connection, Socket ID : " + to_string(tsfd));
	  // can not accept for than max connections
	  if(poll_fd.size() >= (unsigned int)max_connections){
	    LOG(WARNING,"Max connections exceeded dropping new connection");
	    close(tsfd);
	  }
	  // create peer for this socket
	  Peer* p = new Peer();
	  // Resiter Event
	  if(ioctl(tsfd, FIONBIO, (char *)&on) < 0){
	    LOG(ERROR, "Failed to set server socket NON-BLOCKING");
	    close(server_sfd);
	    exit(EXIT_FAILURE);
	  }

	  registerEvent(tsfd, p);
	}while(tsfd != -1);
      }
    }else{
      if(p_fd.events & (POLLHUP|POLLERR)){
	LOG(WARNING,"Connection got disconnected trying again");
	exit(EXIT_FAILURE); //TODO:Need to handle this case as well
      }else if(p_fd.events & (POLLIN)){
	do{
	  numBytesRcvd = read(p_fd.fd, packet_rcvd, sizeof(packet_rcvd));
	  if(numBytesRcvd < 0){						
	    if (errno != EWOULDBLOCK){					
	      close(p_fd.fd);						
	      //TODO: report it to peer as well				
	      unRegisterEvent(p_fd.fd);					
	    }								
            break;							
	  }								
	  // Call handler if i have message 
	  if(numBytesRcvd > 0){
	    LOG(INFO,"Number of bytes Recieved :" + to_string(numBytesRcvd));
	    if((peer = scanEventRegister(p_fd.fd)) == NULL){
	      LOG(WARNING,"Event handler not found for socket" + to_string(p_fd.fd));
	      exit(EXIT_FAILURE); 
	    }
	    peer->readMessage(string(packet_rcvd, numBytesRcvd));
	  }
	  // connection closed
	  if(numBytesRcvd == 0){
	    close(p_fd.fd);
	    //TODO: report it to peer as well 
	    unRegisterEvent(p_fd.fd);
	    break;
	  }
	}while(true);
      }
    }
  }
}

void Reactor::loopForever(){
  
  int ret;
  is_started = true;

  if(!server_sfd){
    LOG(ERROR, "Server socket is not Intialized");
    exit(EXIT_FAILURE);
  }
  // starting reactor
  while(is_started){

    //LOG(DEBUG,"Looping");
    // poll for events
    if((ret = poll( &poll_fd[0],(nfds_t) poll_fd.size(), poll_timeout)) == -1){
      LOG(ERROR, "Polling failed !!");
      exit(EXIT_FAILURE);
    }
    // handle events 
    handleEvent();
    // fill polling fds
    fillPollFd();
  }
}

void Reactor::fillPollFd(){

  pollfd p_fd;
  map<int,Peer*>::iterator it;

  poll_fd.clear();
  //Insert server fd special case
  p_fd.fd = server_sfd;
  p_fd.events = POLLIN;
  poll_fd.push_back(p_fd);
  // Now copy socket descriptors from event register
  for(it = eventRegister.begin(); it != eventRegister.end(); ++it){
    //LOG(DEBUG,"FILLED SOCKET -> " + to_string(it->first));
    p_fd.fd = it->first;
    p_fd.events = POLLIN;
    poll_fd.push_back(p_fd);
  }
}

void Reactor::registerEvent(int fd, Peer* peer){
  m_lock.lock();
  eventRegister.insert( pair<int,Peer*>(fd, peer));
  m_lock.unlock();
  LOG(INFO, "Resigering new socket for POLLIN, socket ID : " + to_string(fd));
}

void Reactor::unRegisterEvent(int fd){
  m_lock.lock();
  eventRegister.erase( fd);
  m_lock.unlock();
  LOG(INFO, "Removed socket  " + to_string(fd) + " from event register ");
}

int Reactor::closeReactor(){
  is_started = false;
  LOG(INFO," Closing reactor");
  return 1;
}

bool Reactor::startReactor()
{
  if(!is_started){
    LOG(INFO,"Starting reactor thread");
    return (pthread_create(&thread, NULL, threadHelper, this) == 0);
  }else{
    LOG(WARNING,"Reactor already started");
    return false;
  }
}

void Reactor::wait(){
  pthread_join(thread,NULL);
}

void* Reactor::threadHelper(void * obj_ptr) {
  ((Reactor*)obj_ptr)->loopForever(); 
  return NULL;
}
